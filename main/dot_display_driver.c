#include "dot_display_driver.h"

#define OP_NOOP        0x00
#define OP_DIGIT0      0x01
#define OP_DIGIT1      0x02
#define OP_DIGIT2      0x03
#define OP_DIGIT3      0x04
#define OP_DIGIT4      0x05
#define OP_DIGIT5      0x06
#define OP_DIGIT6      0x07
#define OP_DIGIT7      0x08
#define OP_DECODEMODE  0x09
#define OP_INTENSITY   0x0a
#define OP_SCANLIMIT   0x0b
#define OP_SHUTDOWN    0x0c
#define OP_DISPLAYTEST 0x0f

static void dot_display_spi_transfer(dot_display_t *display, int addr, volatile uint8_t opcode,	volatile uint8_t data) {
  int offset = addr * 2;
  int maxbytes = display->n_devices * 2;

  uint8_t spidata[16];

  for (int i = 0; i < maxbytes; i++) {
    spidata[i] = (uint8_t) 0;
  }

  spidata[offset] = opcode;
  spidata[offset + 1] = data;
  spi_transfer(display->spi_device, spidata, maxbytes);
}

void dot_display_init(dot_display_t *display, int n_devices, spi_t *spi_device) {
  display->spi_device = spi_device;
  display->n_devices = n_devices;

  for (int i = 0; i < n_devices; i++) {
		dot_display_spi_transfer(display, i, OP_DISPLAYTEST, 0);
    dot_display_spi_transfer(display, i, OP_SCANLIMIT, 7);
		dot_display_spi_transfer(display, i, OP_DECODEMODE, 0);
    dot_display_spi_transfer(display, i, OP_INTENSITY, 15);
		dot_display_clear(display, i);
		dot_display_shutdown(display, i, false);
	}
}

void dot_display_sync(dot_display_t *display, dot_buffer_t *buffer) {
  uint8_t spidata[16];
  int maxbytes = display->n_devices * 2;
  int i, j;

  for (j = OP_DIGIT0; j <= OP_DIGIT7; j++) {
    for (i = 0; i < display->n_devices; i++) {
      spidata[i * 2] = j;
      spidata[i * 2 + 1] = buffer->dots[i + (j - 1) * buffer->n_horiz];
    }

    spi_transfer(display->spi_device, spidata, maxbytes);
  }

  #if CONFIG_MIRROR_EFFECTS_UART
  dot_buffer_print(buffer);
  #endif
}

void dot_display_clear(dot_display_t *display, int addr) {
  if (addr < 0 || addr >= display->n_devices) {
    return;
  }

  for (int i = 0; i < 8; i++) {
    dot_display_spi_transfer(display, addr, i + 1, 0);
  }
}

void dot_display_shutdown(dot_display_t *display, int addr, bool b) {
  if (addr < 0 || addr >= display->n_devices) return;
	if (b) {
		dot_display_spi_transfer(display, addr, OP_SHUTDOWN, 0);
	} else {
		dot_display_spi_transfer(display, addr, OP_SHUTDOWN, 1);
	}
}
