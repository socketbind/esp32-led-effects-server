#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#include "incbin.h"
INCBIN(index, "index.html");
INCBIN(style, "style.css");
INCBIN(font_woff, "font.woff");
INCBIN(font_woff2, "font.woff2");

#define MG_ENABLE_SSL 0
#define MG_ENABLE_IPV6 0
#define MG_ENABLE_MQTT 0
#define MG_ENABLE_MQTT_BROKER 0
#define MG_ENABLE_DNS_SERVER 0
#define MG_ENABLE_COAP 0
#include "mongoose.h"

#define MG_LISTEN_ADDR "80"
#include "http_server_mng.h"
#include "pb_decode.h"
#include "ws.pb.h"

static const char* TAG = "HTTP_Server";

static const char http_hdrs[] =
    "HTTP/1.0 200 OK\r\n"
    "Connection: close\r\n"
    "Content-Type: %s\r\n"
    "\r\n";

static void handle_set_display(SetDisplay *cmd, display_loop_t *loop) {
  ESP_LOGI(TAG, "Set display horiz: %d", cmd->n_horiz);

  if (cmd->dots.size == cmd->n_horiz * 8) {
    display_loop_replace_buffer_safe(loop, cmd->n_horiz, cmd->dots.bytes);
  } else {
    ESP_LOGE(TAG, "Set cmd received buffer with invalid size");
  }
}

static void handle_dot_command(DotCommand *dot_command, display_loop_t *loop) {
  switch (dot_command->which_command) {
    case DotCommand_set_display_tag:
      handle_set_display(&dot_command->command.set_display, loop);
      break;
    default:
      ESP_LOGE(TAG, "Unknown dot command: %d", dot_command->which_command);
  }
}

static void mg_ev_handler(struct mg_connection *nc, int ev, void *p) {
  display_loop_t *current_loop = (display_loop_t*) nc->user_data;

  switch (ev) {
    case MG_EV_WEBSOCKET_FRAME: {
      struct websocket_message *wm = (struct websocket_message *) p;

      DotCommand dot_command = DotCommand_init_zero;
      pb_istream_t stream = pb_istream_from_buffer(wm->data, wm->size);

      bool decode_status = pb_decode(&stream, DotCommand_fields, &dot_command);

      if (decode_status) {
        ESP_LOGI(TAG, "Dot command: %d", dot_command.which_command);
        handle_dot_command(&dot_command, current_loop);
      } else {
        ESP_LOGE(TAG, "Dot command decode failed");
      }

      break;
    }
    case MG_EV_HTTP_REQUEST: {
      struct http_message *hm = (struct http_message *) p;

      char addr[32];
      mg_sock_addr_to_str(&nc->sa, addr, sizeof(addr), MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);
      ESP_LOGI(TAG, "HTTP request from %s: %.*s %.*s", addr, (int) hm->method.len, hm->method.p, (int) hm->uri.len, hm->uri.p);

      if (mg_vcmp(&hm->uri, "/font.woff") == 0) {
        mg_printf(nc, http_hdrs, "font/woff");
        mg_send(nc, gfont_woff_data, gfont_woff_size);
      } else if (mg_vcmp(&hm->uri, "/font.woff2") == 0) {
        mg_printf(nc, http_hdrs, "font/woff2");
        mg_send(nc, gfont_woff2_data, gfont_woff2_size);
      } else if (mg_vcmp(&hm->uri, "/style.css") == 0) {
        mg_printf(nc, http_hdrs, "text/css");
        mg_send(nc, gstyle_data, gstyle_size);
      } else if (mg_vcmp(&hm->uri, "/") == 0) {
        mg_printf(nc, http_hdrs, "text/html");
        mg_send(nc, gindex_data, gindex_size);

        if (mg_vcmp(&hm->method, "POST") == 0) {
          char text_var[100];
          if (mg_get_http_var(&hm->body, "text", text_var, sizeof(text_var)) != 0) {
            display_loop_change_text_safe(current_loop, text_var);
          }

          char effect_num_var[6];
          if (mg_get_http_var(&hm->body, "effect", effect_num_var, sizeof(effect_num_var)) != 0) {
            int effect_num = atoi(effect_num_var);

            display_loop_change_effect_safe(current_loop, effect_num);
          }
        }
      } else {
        mg_printf(nc, "%s",
          "HTTP/1.1 404 Not Found\r\n"
          "Content-Length: 0\r\n\r\n");
      }
      
      nc->flags |= MG_F_SEND_AND_CLOSE;
      break;
    }
  }
}

static void http_server_task(void *pvParameter) {
  display_loop_t *current_loop = (display_loop_t*) pvParameter;

  struct mg_mgr mgr;
  struct mg_connection *nc;

  ESP_LOGI(TAG, "Starting web-server on port %s", MG_LISTEN_ADDR);

  mg_mgr_init(&mgr, NULL);

  nc = mg_bind(&mgr, MG_LISTEN_ADDR, mg_ev_handler);
  if (nc == NULL) {
    ESP_LOGE(TAG, "Error setting up listener!");
    return;
  }
  mg_set_protocol_http_websocket(nc);
  nc->user_data = current_loop;

  while (1) {
    mg_mgr_poll(&mgr, 1000);
  }

  mg_mgr_free(&mgr);
}

void http_server_mng_init(display_loop_t *loop) {
  xTaskCreate(&http_server_task, "http_server_task", 4096, loop, 5, NULL);
}
