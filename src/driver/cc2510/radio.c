#include "radio.h"

#include "debug.h"
#include "delay.h"
#include "dma.h"

#include "redpine.h"

#define RADIO_RX_BUF_SIZE REDPINE_PACKET_BUFFER_SIZE

#ifdef RF_LNA_PORT
static void lna_enable() {
  PORT2BIT(RF_LNA_PORT, RF_LNA_PIN) = RF_LNA_ON_LEVEL;
}

static void lna_disable() {
  PORT2BIT(RF_LNA_PORT, RF_LNA_PIN) = ~RF_LNA_ON_LEVEL;
}
#ifdef RF_PA_PORT
static void pa_enable() {
  PORT2BIT(RF_PA_PORT, RF_PA_PIN) = RF_PA_ON_LEVEL;
}
static void pa_disable() {
  PORT2BIT(RF_PA_PORT, RF_PA_PIN) = ~RF_PA_ON_LEVEL;
}
#endif
#endif

volatile __xdata uint8_t packet[RADIO_RX_BUF_SIZE];
static volatile uint8_t received_packet = 0;

uint8_t current_antenna = 0;
static void set_antenna(uint8_t index) {
#ifdef RF_ANTENNA_SWITCH_PORT
  if (index == 0) {
    PORT2BIT(RF_ANTENNA_SWITCH_PORT, RF_ANTENNA_SWITCH_PIN) = RF_ANTENNA_A_LEVEL;
#ifdef RF_ANTENNA_SWITCH_PORT2
    PORT2BIT(RF_ANTENNA_SWITCH_PORT2, RF_ANTENNA_SWITCH_PIN2) = ~RF_ANTENNA_A_LEVEL2;
#endif
  } else {
    PORT2BIT(RF_ANTENNA_SWITCH_PORT, RF_ANTENNA_SWITCH_PIN) = ~RF_ANTENNA_A_LEVEL;
#ifdef RF_ANTENNA_SWITCH_PORT2
    PORT2BIT(RF_ANTENNA_SWITCH_PORT2, RF_ANTENNA_SWITCH_PIN2) = RF_ANTENNA_A_LEVEL2;
#endif
  }

  current_antenna = index;
#endif
}

void radio_init() {
  debug_print("radio_init\r\n");

#ifdef RF_LNA_PORT
  PORT2DIR(RF_LNA_PORT) |= (1 << RF_LNA_PIN);
#ifdef USE_LNA
  lna_enable();
#else
  lna_disable();
#endif
#endif

#ifdef RF_PA_PORT
  PORT2DIR(RF_PA_PORT) |= (1 << RF_PA_PIN);
  pa_disable();
#endif

#ifdef RF_ANTENNA_SWITCH_PORT
  PORT2DIR(RF_ANTENNA_SWITCH_PORT) |= (1 << RF_ANTENNA_SWITCH_PIN);
#ifdef RF_ANTENNA_SWITCH_PORT2
  PORT2DIR(RF_ANTENNA_SWITCH_PORT2) |= (1 << RF_ANTENNA_SWITCH_PIN2);
#endif
#endif
  set_antenna(0);

  IP1 |= (1 << 0);
  IP0 |= (1 << 0);

  DMAARM = DMA_ABORT | DMA_CH0;
  delay_us(15);

  WRITE_WORD(dma_desc[0].SRCADDRH, dma_desc[0].SRCADDRL, &X_RFD);
  WRITE_WORD(dma_desc[0].DESTADDRH, dma_desc[0].DESTADDRL, packet);
  WRITE_WORD(dma_desc[0].LENH, dma_desc[0].LENL, RADIO_RX_BUF_SIZE);

  dma_desc[0].VLEN = 0x04;
  dma_desc[0].WORDSIZE = 0x0;
  dma_desc[0].TMODE = 0x0;

  dma_desc[0].TRIG = 19;
  dma_desc[0].SRCINC = 0x0;
  dma_desc[0].DESTINC = 0x1;

  dma_desc[0].IRQMASK = 0x0;
  dma_desc[0].M8 = 0x0;
  dma_desc[0].PRIORITY = 0x02;

  IEN2 |= (1 << 0);
  RFIM = (1 << 4);
}

uint8_t radio_received_packet() {
  return received_packet;
}

void radio_reset_packet() {
  received_packet = 0;
}

void radio_switch_antenna() {
#ifdef USE_FIXED_ANTENNA
  set_antenna(USE_FIXED_ANTENNA);
#else
  set_antenna(current_antenna == 0 ? 1 : 0);
#endif
}

void radio_enable_rx() {
#ifdef RF_LNA_PORT
#ifdef USE_LNA
  lna_enable();
#else
  lna_disable();
#endif
#endif
#ifdef RF_PA_PORT
  pa_disable();
#endif

  if ((DMAARM & DMA_CH0) == 0) {
    DMAARM |= DMA_CH0;
  }
}

void radio_isr(void) __interrupt(RF_VECTOR) {
  RFIF &= ~(1 << 4);
  S1CON &= ~0x03;

  radio_enable_rx();
  received_packet = 1;
}

void radio_handle_overflows() {
  uint8_t marc_state = radio_read_reg(MARCSTATE) & 0x1F;
  if (marc_state == 0x11) {
    radio_strobe(RFST_SIDLE);
    while (radio_read_reg(MARCSTATE) != 0x01)
      ;

    radio_reset_packet();
    radio_enable_rx();

    radio_strobe(RFST_SRX);
  }
}