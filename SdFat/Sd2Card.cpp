/* Arduino Sd2Card Library
 * Copyright (C) 2012 by William Greiman
 *
 * This file is part of the Arduino Sd2Card Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino Sd2Card Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include <Sd2Card.h>
// debug trace macro
#define SD_TRACE(m, b)
// #define SD_TRACE(m, b) Serial.print(m);Serial.println(b);

// SPI functions
//==============================================================================
#if USE_ARDUINO_SPI_LIBRARY
#include <SPI.h>
//------------------------------------------------------------------------------
static void spiBegin() {
  SPI.begin();
}
//------------------------------------------------------------------------------
static void spiInit(uint8_t spiRate) {
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  int v;
#ifdef SPI_CLOCK_DIV128
  switch (spiRate/2) {
    case 0: v = SPI_CLOCK_DIV2; break;
    case 1: v = SPI_CLOCK_DIV4; break;
    case 2: v = SPI_CLOCK_DIV8; break;
    case 3: v = SPI_CLOCK_DIV16; break;
    case 4: v = SPI_CLOCK_DIV32; break;
    case 5: v = SPI_CLOCK_DIV64; break;
    default: v = SPI_CLOCK_DIV128; break;
  }
#else  // SPI_CLOCK_DIV128
  if (spiRate > 13) {
    v = 255;
  } else {
    v = (2 | (spiRate & 1)) << (spiRate/2);
  }
#endif  // SPI_CLOCK_DIV128
  SPI.setClockDivider(v);
}
//------------------------------------------------------------------------------
/** SPI receive a byte */
static  uint8_t spiRec() {
  return SPI.transfer(0XFF);
}
//------------------------------------------------------------------------------
/** SPI receive multiple bytes */
static uint8_t spiRec(uint8_t* buf, size_t len) {
  for (size_t i = 0; i < len; i++) {
    buf[i] = SPI.transfer(0XFF);
  }
  return 0;
}
//------------------------------------------------------------------------------
/** SPI send a byte */
static void spiSend(uint8_t b) {
  SPI.transfer(b);
}
//------------------------------------------------------------------------------
/** SPI send multiple bytes */
static void spiSend(const uint8_t* buf, size_t len) {
  for (size_t i = 0; i < len; i++) {
    SPI.transfer(buf[i]);
  }
}
//==============================================================================
#elif USE_NATIVE_SAM3X_SPI
/** Use SAM3X DMAC if nonzero */
#define USE_SAM3X_DMAC 1
/** Use extra Bus Matrix arbitration fix if nonzero */
#define USE_SAM3X_BUS_MATRIX_FIX 0
/** Time in ms for DMA receive timeout */
#define SAM3X_DMA_TIMEOUT 100
/** chip select register number */
#define SPI_CHIP_SEL 3
/** DMAC receive channel */
#define SPI_DMAC_RX_CH  1
/** DMAC transmit channel */
#define SPI_DMAC_TX_CH  0
/** DMAC Channel HW Interface Number for SPI TX. */
#define SPI_TX_IDX  1
/** DMAC Channel HW Interface Number for SPI RX. */
#define SPI_RX_IDX  2
//------------------------------------------------------------------------------
/** Disable DMA Controller. */
static void dmac_disable() {
  DMAC->DMAC_EN &= (~DMAC_EN_ENABLE);
}
/** Enable DMA Controller. */
static void dmac_enable() {
  DMAC->DMAC_EN = DMAC_EN_ENABLE;
}
/** Disable DMA Channel. */
static void dmac_channel_disable(uint32_t ul_num) {
  DMAC->DMAC_CHDR = DMAC_CHDR_DIS0 << ul_num;
}
/** Enable DMA Channel. */
static void dmac_channel_enable(uint32_t ul_num) {
  DMAC->DMAC_CHER = DMAC_CHER_ENA0 << ul_num;
}
/** Poll for transfer complete. */
static bool dmac_channel_transfer_done(uint32_t ul_num) {
  return (DMAC->DMAC_CHSR & (DMAC_CHSR_ENA0 << ul_num)) ? false : true;
}
//------------------------------------------------------------------------------
static void spiBegin() {
  PIO_Configure(
      g_APinDescription[PIN_SPI_MOSI].pPort,
      g_APinDescription[PIN_SPI_MOSI].ulPinType,
      g_APinDescription[PIN_SPI_MOSI].ulPin,
      g_APinDescription[PIN_SPI_MOSI].ulPinConfiguration);
  PIO_Configure(
      g_APinDescription[PIN_SPI_MISO].pPort,
      g_APinDescription[PIN_SPI_MISO].ulPinType,
      g_APinDescription[PIN_SPI_MISO].ulPin,
      g_APinDescription[PIN_SPI_MISO].ulPinConfiguration);
  PIO_Configure(
      g_APinDescription[PIN_SPI_SCK].pPort,
      g_APinDescription[PIN_SPI_SCK].ulPinType,
      g_APinDescription[PIN_SPI_SCK].ulPin,
      g_APinDescription[PIN_SPI_SCK].ulPinConfiguration);
  pmc_enable_periph_clk(ID_SPI0);
#if USE_SAM3X_DMAC
  pmc_enable_periph_clk(ID_DMAC);
  dmac_disable();
  DMAC->DMAC_GCFG = DMAC_GCFG_ARB_CFG_FIXED;
  dmac_enable();
#if USE_SAM3X_BUS_MATRIX_FIX
  MATRIX->MATRIX_WPMR = 0x4d415400;
  MATRIX->MATRIX_MCFG[1] = 1;
  MATRIX->MATRIX_MCFG[2] = 1;
  MATRIX->MATRIX_SCFG[0] = 0x01000010;
  MATRIX->MATRIX_SCFG[1] = 0x01000010;
  MATRIX->MATRIX_SCFG[7] = 0x01000010;
#endif  // USE_SAM3X_BUS_MATRIX_FIX
#endif  // USE_SAM3X_DMAC
}
//------------------------------------------------------------------------------
// start RX DMA
void spiDmaRX(uint8_t* dst, uint16_t count) {
  dmac_channel_disable(SPI_DMAC_RX_CH);
  DMAC->DMAC_CH_NUM[SPI_DMAC_RX_CH].DMAC_SADDR = (uint32_t)&SPI0->SPI_RDR;
  DMAC->DMAC_CH_NUM[SPI_DMAC_RX_CH].DMAC_DADDR = (uint32_t)dst;
  DMAC->DMAC_CH_NUM[SPI_DMAC_RX_CH].DMAC_DSCR =  0;
  DMAC->DMAC_CH_NUM[SPI_DMAC_RX_CH].DMAC_CTRLA = count |
    DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;
  DMAC->DMAC_CH_NUM[SPI_DMAC_RX_CH].DMAC_CTRLB = DMAC_CTRLB_SRC_DSCR |
    DMAC_CTRLB_DST_DSCR | DMAC_CTRLB_FC_PER2MEM_DMA_FC |
    DMAC_CTRLB_SRC_INCR_FIXED | DMAC_CTRLB_DST_INCR_INCREMENTING;
  DMAC->DMAC_CH_NUM[SPI_DMAC_RX_CH].DMAC_CFG = DMAC_CFG_SRC_PER(SPI_RX_IDX) |
    DMAC_CFG_SRC_H2SEL | DMAC_CFG_SOD | DMAC_CFG_FIFOCFG_ASAP_CFG;
  dmac_channel_enable(SPI_DMAC_RX_CH);
}
//------------------------------------------------------------------------------
// start TX DMA
void spiDmaTX(const uint8_t* src, uint16_t count) {
  static uint8_t ff = 0XFF;
  uint32_t src_incr = DMAC_CTRLB_SRC_INCR_INCREMENTING;
  if (!src) {
    src = &ff;
    src_incr = DMAC_CTRLB_SRC_INCR_FIXED;
  }
  dmac_channel_disable(SPI_DMAC_TX_CH);
  DMAC->DMAC_CH_NUM[SPI_DMAC_TX_CH].DMAC_SADDR = (uint32_t)src;
  DMAC->DMAC_CH_NUM[SPI_DMAC_TX_CH].DMAC_DADDR = (uint32_t)&SPI0->SPI_TDR;
  DMAC->DMAC_CH_NUM[SPI_DMAC_TX_CH].DMAC_DSCR =  0;
  DMAC->DMAC_CH_NUM[SPI_DMAC_TX_CH].DMAC_CTRLA = count |
    DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE;

  DMAC->DMAC_CH_NUM[SPI_DMAC_TX_CH].DMAC_CTRLB =  DMAC_CTRLB_SRC_DSCR |
    DMAC_CTRLB_DST_DSCR | DMAC_CTRLB_FC_MEM2PER_DMA_FC |
    src_incr | DMAC_CTRLB_DST_INCR_FIXED;

  DMAC->DMAC_CH_NUM[SPI_DMAC_TX_CH].DMAC_CFG = DMAC_CFG_DST_PER(SPI_TX_IDX) |
      DMAC_CFG_DST_H2SEL | DMAC_CFG_SOD | DMAC_CFG_FIFOCFG_ALAP_CFG;

  dmac_channel_enable(SPI_DMAC_TX_CH);
}
//------------------------------------------------------------------------------
//  initialize SPI controller
static void spiInit(uint8_t spiRate) {
  Spi* pSpi = SPI0;
  uint8_t scbr = 255;
  if (spiRate < 14) {
    scbr = (2 | (spiRate & 1)) << (spiRate/2);
  }
  //  disable SPI
  pSpi->SPI_CR = SPI_CR_SPIDIS;
  // reset SPI
  pSpi->SPI_CR = SPI_CR_SWRST;
  // no mode fault detection, set master mode
  pSpi->SPI_MR = SPI_PCS(SPI_CHIP_SEL) | SPI_MR_MODFDIS | SPI_MR_MSTR;
  // mode 0, 8-bit,
  pSpi->SPI_CSR[SPI_CHIP_SEL] = SPI_CSR_SCBR(scbr) | SPI_CSR_NCPHA;
  // enable SPI
  pSpi->SPI_CR |= SPI_CR_SPIEN;
}
//------------------------------------------------------------------------------
static inline uint8_t spiTransfer(uint8_t b) {
  Spi* pSpi = SPI0;

  pSpi->SPI_TDR = b;
  while ((pSpi->SPI_SR & SPI_SR_RDRF) == 0) {}
  b = pSpi->SPI_RDR;
  return b;
}
//------------------------------------------------------------------------------
/** SPI receive a byte */
static inline uint8_t spiRec() {
  return spiTransfer(0XFF);
}
//------------------------------------------------------------------------------
/** SPI receive multiple bytes */
static uint8_t spiRec(uint8_t* buf, size_t len) {
  Spi* pSpi = SPI0;
  int rtn = 0;
#if USE_SAM3X_DMAC
  // clear overrun error
  uint32_t s = pSpi->SPI_SR;

  spiDmaRX(buf, len);
  spiDmaTX(0, len);

  uint32_t m = millis();
  while (!dmac_channel_transfer_done(SPI_DMAC_RX_CH)) {
    if ((millis() - m) > SAM3X_DMA_TIMEOUT)  {
      dmac_channel_disable(SPI_DMAC_RX_CH);
      dmac_channel_disable(SPI_DMAC_TX_CH);
      rtn = 2;
      break;
    }
  }
  if (pSpi->SPI_SR & SPI_SR_OVRES) rtn |= 1;
#else  // USE_SAM3X_DMAC
  for (size_t i = 0; i < len; i++) {
    pSpi->SPI_TDR = 0XFF;
    while ((pSpi->SPI_SR & SPI_SR_RDRF) == 0) {}
    buf[i] = pSpi->SPI_RDR;
  }
#endif  // USE_SAM3X_DMAC
  return rtn;
}
//------------------------------------------------------------------------------
/** SPI send a byte */
static inline void spiSend(uint8_t b) {
  spiTransfer(b);
}
//------------------------------------------------------------------------------
static void spiSend(const uint8_t* buf, size_t len) {
  Spi* pSpi = SPI0;
#if USE_SAM3X_DMAC
  spiDmaTX(buf, len);
  while (!dmac_channel_transfer_done(SPI_DMAC_TX_CH)) {}
#else  // #if USE_SAM3X_DMAC
  while ((pSpi->SPI_SR & SPI_SR_TXEMPTY) == 0) {}
  for (size_t i = 0; i < len; i++) {
    pSpi->SPI_TDR = buf[i];
    while ((pSpi->SPI_SR & SPI_SR_TDRE) == 0) {}
  }
#endif  // #if USE_SAM3X_DMAC
  while ((pSpi->SPI_SR & SPI_SR_TXEMPTY) == 0) {}
  // leave RDR empty
  uint8_t b = pSpi->SPI_RDR;
}
//==============================================================================
#elif USE_NATIVE_MK20DX128_SPI
// Teensy 3.0 functions
#include <mk20dx128.h>
// use 16-bit frame if SPI_USE_8BIT_FRAME is zero
#define SPI_USE_8BIT_FRAME 0
// Limit initial fifo to three entries to avoid fifo overrun
#define SPI_INITIAL_FIFO_DEPTH 3
// define some symbols that are not in mk20dx128.h
#ifndef SPI_SR_RXCTR
#define SPI_SR_RXCTR 0XF0
#endif  // SPI_SR_RXCTR
#ifndef SPI_PUSHR_CONT
#define SPI_PUSHR_CONT 0X80000000
#endif   // SPI_PUSHR_CONT
#ifndef SPI_PUSHR_CTAS
#define SPI_PUSHR_CTAS(n) (((n) & 7) << 28)
#endif  // SPI_PUSHR_CTAS

//------------------------------------------------------------------------------
/**
 * initialize SPI pins
 */
static void spiBegin() {
  SIM_SCGC6 |= SIM_SCGC6_SPI0;
}
//------------------------------------------------------------------------------
/**
 * Initialize hardware SPI
 *
 */
static void spiInit(uint8_t spiRate) {
  // spiRate = 0 or 1 : 24 or 12 Mbit/sec
  // spiRate = 2 or 3 : 12 or 6 Mbit/sec
  // spiRate = 4 or 5 : 6 or 3 Mbit/sec
  // spiRate = 6 or 7 : 4 or 2.0 Mbit/sec
  // spiRate = 8 or 9 : 3 or 1.5 Mbit/sec
  // spiRate = 10 or 11 : 250 kbit/sec
  // spiRate = 12 or more : 125 kbit/sec
  uint32_t ctar, ctar0, ctar1;
  switch (spiRate/2) {
    // 1/2 speed
    case 0: ctar = SPI_CTAR_DBR | SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0); break;
    // 1/4 speed
    case 1: ctar = SPI_CTAR_BR(0) | SPI_CTAR_CSSCK(0); break;
    // 1/8 speed
    case 2: ctar = SPI_CTAR_BR(1) | SPI_CTAR_CSSCK(1); break;
    // 1/12 speed
    case 3: ctar = SPI_CTAR_BR(2) | SPI_CTAR_CSSCK(2); break;
    // 1/16 speed
    case 4: ctar = SPI_CTAR_BR(3) | SPI_CTAR_CSSCK(3); break;
#if F_BUS == 48000000
    case 5: ctar = SPI_CTAR_PBR(1) | SPI_CTAR_BR(5) | SPI_CTAR_CSSCK(5); break;
    default: ctar = SPI_CTAR_PBR(1) | SPI_CTAR_BR(6) | SPI_CTAR_CSSCK(6);
#elif F_BUS == 24000000
    case 5: ctar = SPI_CTAR_PBR(1) | SPI_CTAR_BR(4) | SPI_CTAR_CSSCK(4); break;
    default: ctar = SPI_CTAR_PBR(1) | SPI_CTAR_BR(5) | SPI_CTAR_CSSCK(5);
#else
#error "MK20DX128 bus frequency must be 48 or 24 MHz"
#endif
  }

  // CTAR0 - 8 bit transfer
  ctar0 = ctar | SPI_CTAR_FMSZ(7);
  // CTAR1 - 16 bit transfer
  ctar1 = ctar | SPI_CTAR_FMSZ(15);

  if (SPI0_CTAR0 != ctar0 || SPI0_CTAR1 != ctar1) {
    SPI0_MCR = SPI_MCR_MSTR | SPI_MCR_MDIS | SPI_MCR_HALT;
    SPI0_CTAR0 = ctar0;
    SPI0_CTAR1 = ctar1;
  }
  SPI0_MCR = SPI_MCR_MSTR;
  CORE_PIN11_CONFIG = PORT_PCR_DSE | PORT_PCR_MUX(2);
  CORE_PIN12_CONFIG = PORT_PCR_MUX(2);
  CORE_PIN13_CONFIG = PORT_PCR_DSE | PORT_PCR_MUX(2);
}
//------------------------------------------------------------------------------
/** SPI receive a byte */
static  uint8_t spiRec() {
  SPI0_MCR |= SPI_MCR_CLR_RXF;
  SPI0_SR = SPI_SR_TCF;
  SPI0_PUSHR = 0xFF;
  while (!(SPI0_SR & SPI_SR_TCF)) {}
  return SPI0_POPR;
}
//------------------------------------------------------------------------------
/** SPI receive multiple bytes */
static uint8_t spiRec(uint8_t* buf, size_t len) {
  // clear any data in RX FIFO
  SPI0_MCR = SPI_MCR_MSTR | SPI_MCR_CLR_RXF;
#if SPI_USE_8BIT_FRAME
  // initial number of bytes to push into TX FIFO
  int nf = len < SPI_INITIAL_FIFO_DEPTH ? len : SPI_INITIAL_FIFO_DEPTH;
  for (int i = 0; i < nf; i++) {
    SPI0_PUSHR = 0XFF;
  }
  // limit for pushing dummy data into TX FIFO
  uint8_t* limit = buf + len - nf;
  while (buf < limit) {
    while (!(SPI0_SR & SPI_SR_RXCTR)) {}
    SPI0_PUSHR = 0XFF;
    *buf++ = SPI0_POPR;
  }
  // limit for rest of RX data
  limit += nf;
  while (buf < limit) {
    while (!(SPI0_SR & SPI_SR_RXCTR)) {}
    *buf++ = SPI0_POPR;
  }
#else  // SPI_USE_8BIT_FRAME
  // use 16 bit frame to avoid TD delay between frames
  // get one byte if len is odd
  if (len & 1) {
    *buf++ = spiRec();
    len--;
  }
  // initial number of words to push into TX FIFO
  int nf = len/2 < SPI_INITIAL_FIFO_DEPTH ? len/2 : SPI_INITIAL_FIFO_DEPTH;
  for (int i = 0; i < nf; i++) {
    SPI0_PUSHR = SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1) | 0XFFFF;
  }
  uint8_t* limit = buf + len - 2*nf;
  while (buf < limit) {
    while (!(SPI0_SR & SPI_SR_RXCTR)) {}
    SPI0_PUSHR = SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1) | 0XFFFF;
    uint16_t w = SPI0_POPR;
    *buf++ = w >> 8;
    *buf++ = w & 0XFF;
  }
  // limit for rest of RX data
  limit += 2*nf;
  while (buf < limit) {
    while (!(SPI0_SR & SPI_SR_RXCTR)) {}
    uint16_t w = SPI0_POPR;
    *buf++ = w >> 8;
    *buf++ = w & 0XFF;
  }
#endif  // SPI_USE_8BIT_FRAME
  return 0;
}
//------------------------------------------------------------------------------
/** SPI send a byte */
static void spiSend(uint8_t b) {
  SPI0_MCR |= SPI_MCR_CLR_RXF;
  SPI0_SR = SPI_SR_TCF;
  SPI0_PUSHR = b;
  while (!(SPI0_SR & SPI_SR_TCF)) {}
}
//------------------------------------------------------------------------------
/** SPI send multiple bytes */
static void spiSend(const uint8_t* output, size_t len) {
  // clear any data in RX FIFO
  SPI0_MCR = SPI_MCR_MSTR | SPI_MCR_CLR_RXF;
#if SPI_USE_8BIT_FRAME
  // initial number of bytes to push into TX FIFO
  int nf = len < SPI_INITIAL_FIFO_DEPTH ? len : SPI_INITIAL_FIFO_DEPTH;
  // limit for pushing data into TX fifo
  const uint8_t* limit = output + len;
  for (int i = 0; i < nf; i++) {
    SPI0_PUSHR = *output++;
  }
  // write data to TX FIFO
  while (output < limit) {
    while (!(SPI0_SR & SPI_SR_RXCTR)) {}
    SPI0_PUSHR = *output++;
    SPI0_POPR;
  }
  // wait for data to be sent
  while (nf) {
    while (!(SPI0_SR & SPI_SR_RXCTR)) {}
    SPI0_POPR;
    nf--;
  }
#else  // SPI_USE_8BIT_FRAME
  // use 16 bit frame to avoid TD delay between frames
  // send one byte if len is odd
  if (len & 1) {
    spiSend(*output++);
    len--;
  }
  // initial number of words to push into TX FIFO
  int nf = len/2 < SPI_INITIAL_FIFO_DEPTH ? len/2 : SPI_INITIAL_FIFO_DEPTH;
  // limit for pushing data into TX fifo
  const uint8_t* limit = output + len;
  for (int i = 0; i < nf; i++) {
    uint16_t w = (*output++) << 8;
    w |= *output++;
    SPI0_PUSHR = SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1) | w;
  }
  // write data to TX FIFO
  while (output < limit) {
    uint16_t w = *output++ << 8;
    w |= *output++;
    while (!(SPI0_SR & SPI_SR_RXCTR)) {}
    SPI0_PUSHR = SPI_PUSHR_CONT | SPI_PUSHR_CTAS(1) | w;
    SPI0_POPR;
  }
  // wait for data to be sent
  while (nf) {
    while (!(SPI0_SR & SPI_SR_RXCTR)) {}
    SPI0_POPR;
    nf--;
  }
#endif  // SPI_USE_8BIT_FRAME
}
//==============================================================================
#elif defined(SOFTWARE_SPI)
#include <SoftSPI.h>
static
SoftSPI<SOFT_SPI_MISO_PIN, SOFT_SPI_MOSI_PIN, SOFT_SPI_SCK_PIN, 0> softSpiBus;
//------------------------------------------------------------------------------
/**
 * initialize SPI pins
 */
static void spiBegin() {
  softSpiBus.begin();
}
//------------------------------------------------------------------------------
/**
 * Initialize hardware SPI - dummy for soft SPI
 */
static void spiInit(uint8_t spiRate) {}
//------------------------------------------------------------------------------
/** Soft SPI receive byte */
static uint8_t spiRec() {
  return softSpiBus.receive();
}
//------------------------------------------------------------------------------
/** Soft SPI read data */
static uint8_t spiRec(uint8_t* buf, size_t n) {
  for (size_t i = 0; i < n; i++) {
    buf[i] = spiRec();
  }
  return 0;
}
//------------------------------------------------------------------------------
/** Soft SPI send byte */
static void spiSend(uint8_t data) {
  softSpiBus.send(data);
}
//------------------------------------------------------------------------------
static void spiSend(const uint8_t* buf, size_t n) {
  for (size_t i = 0; i < n; i++) {
    spiSend(buf[i]);
  }
}
//==============================================================================
#else
// functions for AVR hardware SPI
//------------------------------------------------------------------------------
// make sure SPCR rate is in expected bits
#if (SPR0 != 0 || SPR1 != 1)
#error unexpected SPCR bits
#endif
//------------------------------------------------------------------------------
/**
 * initialize SPI pins
 */
static void spiBegin() {
  // set SS high - may be chip select for another SPI device
  digitalWrite(SS, HIGH);
  // SS must be in output mode even it is not chip select
  pinMode(SS, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(SCK, OUTPUT);
}
//------------------------------------------------------------------------------
/**
 * Initialize hardware SPI
 * Set SCK rate to F_CPU/pow(2, 1 + spiRate) for spiRate [0,6]
 */
static void spiInit(uint8_t spiRate) {
  spiRate = spiRate > 12 ? 6 : spiRate/2;
  // See avr processor documentation
  SPCR = (1 << SPE) | (1 << MSTR) | (spiRate >> 1);
  SPSR = spiRate & 1 || spiRate == 6 ? 0 : 1 << SPI2X;
}
//------------------------------------------------------------------------------
/** SPI receive a byte */
static  uint8_t spiRec() {
  SPDR = 0XFF;
  while (!(SPSR & (1 << SPIF)));
  return SPDR;
}
//------------------------------------------------------------------------------
/** SPI receive multiple bytes */
static uint8_t spiRec(uint8_t* buf, size_t n) {
  if (n-- == 0) return 0;
  SPDR = 0XFF;
  for (size_t i = 0; i < n; i++) {
    while (!(SPSR & (1 << SPIF)));
    uint8_t b = SPDR;
    SPDR = 0XFF;
    buf[i] = b;
  }
  while (!(SPSR & (1 << SPIF)));
  buf[n] = SPDR;
  return 0;
}
//------------------------------------------------------------------------------
/** SPI send a byte */
static void spiSend(uint8_t b) {
  SPDR = b;
  while (!(SPSR & (1 << SPIF)));
}
//------------------------------------------------------------------------------
static void spiSend(const uint8_t* buf , size_t n) {
  if (n == 0) return;
  SPDR = buf[0];
  if (n > 1) {
    uint8_t b = buf[1];
    size_t i = 2;
    while (1) {
      while (!(SPSR & (1 << SPIF)));
      SPDR = b;
      if (i == n) break;
      b = buf[i++];
    }
  }
  while (!(SPSR & (1 << SPIF)));
}
#endif  // SOFTWARE_SPI
//==============================================================================
#if USE_SD_CRC
// CRC functions
//------------------------------------------------------------------------------
static uint8_t CRC7(const uint8_t* data, uint8_t n) {
  uint8_t crc = 0;
  for (uint8_t i = 0; i < n; i++) {
    uint8_t d = data[i];
    for (uint8_t j = 0; j < 8; j++) {
      crc <<= 1;
      if ((d & 0x80) ^ (crc & 0x80)) crc ^= 0x09;
      d <<= 1;
    }
  }
  return (crc << 1) | 1;
}
//------------------------------------------------------------------------------
#if USE_SD_CRC == 1
// slower CRC-CCITT
// uses the x^16,x^12,x^5,x^1 polynomial.
static uint16_t CRC_CCITT(const uint8_t *data, size_t n) {
  uint16_t crc = 0;
  for (size_t i = 0; i < n; i++) {
    crc = (uint8_t)(crc >> 8) | (crc << 8);
    crc ^= data[i];
    crc ^= (uint8_t)(crc & 0xff) >> 4;
    crc ^= crc << 12;
    crc ^= (crc & 0xff) << 5;
  }
  return crc;
}
#elif USE_SD_CRC > 1  // CRC_CCITT
//------------------------------------------------------------------------------
// faster CRC-CCITT
// uses the x^16,x^12,x^5,x^1 polynomial.
#ifdef __AVR__
static const uint16_t crctab[] PROGMEM = {
#else  // __AVR__
static const uint16_t crctab[] = {
#endif  // __AVR__
  0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
  0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
  0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
  0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
  0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
  0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
  0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
  0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
  0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
  0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
  0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
  0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
  0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
  0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
  0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
  0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
  0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
  0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
  0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
  0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
  0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
  0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
  0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
  0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
  0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
  0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
  0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
  0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
  0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
  0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
  0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
  0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};
static uint16_t CRC_CCITT(const uint8_t* data, size_t n) {
  uint16_t crc = 0;
  for (size_t i = 0; i < n; i++) {
#ifdef __AVR__
    crc = pgm_read_word(&crctab[(crc >> 8 ^ data[i]) & 0XFF]) ^ (crc << 8);
#else  // __AVR__
    crc = crctab[(crc >> 8 ^ data[i]) & 0XFF] ^ (crc << 8);
#endif  // __AVR__
  }
  return crc;
}
#endif  // CRC_CCITT
#endif  // USE_SD_CRC
//==============================================================================
// Sd2Card member functions
//------------------------------------------------------------------------------
// send command and return error code.  Return zero for OK
uint8_t Sd2Card::cardCommand(uint8_t cmd, uint32_t arg) {
  // select card
  chipSelectLow();

  // wait up to 300 ms if busy
  waitNotBusy(300);

  uint8_t *pa = reinterpret_cast<uint8_t *>(&arg);

#if USE_SD_CRC
  // form message
  uint8_t d[6] = {cmd | 0X40, pa[3], pa[2], pa[1], pa[0]};

  // add crc
  d[5] = CRC7(d, 5);

  // send message
  for (uint8_t k = 0; k < 6; k++) spiSend(d[k]);
#else  // USE_SD_CRC
  // send command
  spiSend(cmd | 0x40);

  // send argument
  for (int8_t i = 3; i >= 0; i--) spiSend(pa[i]);

  // send CRC - correct for CMD0 with arg zero or CMD8 with arg 0X1AA
  spiSend(cmd == CMD0 ? 0X95 : 0X87);
#endif  // USE_SD_CRC

  // skip stuff byte for stop read
  if (cmd == CMD12) spiRec();

  // wait for response
  for (uint8_t i = 0; ((status_ = spiRec()) & 0X80) && i != 0XFF; i++);
  return status_;
}
//------------------------------------------------------------------------------
/**
 * Determine the size of an SD flash memory card.
 *
 * \return The number of 512 byte data blocks in the card
 *         or zero if an error occurs.
 */
uint32_t Sd2Card::cardSize() {
  csd_t csd;
  if (!readCSD(&csd)) return 0;
  if (csd.v1.csd_ver == 0) {
    uint8_t read_bl_len = csd.v1.read_bl_len;
    uint16_t c_size = (csd.v1.c_size_high << 10)
                      | (csd.v1.c_size_mid << 2) | csd.v1.c_size_low;
    uint8_t c_size_mult = (csd.v1.c_size_mult_high << 1)
                          | csd.v1.c_size_mult_low;
    return (uint32_t)(c_size + 1) << (c_size_mult + read_bl_len - 7);
  } else if (csd.v2.csd_ver == 1) {
    uint32_t c_size = 0X10000L * csd.v2.c_size_high + 0X100L
                      * (uint32_t)csd.v2.c_size_mid + csd.v2.c_size_low;
    return (c_size + 1) << 10;
  } else {
    error(SD_CARD_ERROR_BAD_CSD);
    return 0;
  }
}
//------------------------------------------------------------------------------
void Sd2Card::chipSelectHigh() {
  digitalWrite(chipSelectPin_, HIGH);
  // insure MISO goes high impedance
  spiSend(0XFF);
}
//------------------------------------------------------------------------------
void Sd2Card::chipSelectLow() {
  spiInit(spiRate_);
  digitalWrite(chipSelectPin_, LOW);
}
//------------------------------------------------------------------------------
/** Erase a range of blocks.
 *
 * \param[in] firstBlock The address of the first block in the range.
 * \param[in] lastBlock The address of the last block in the range.
 *
 * \note This function requests the SD card to do a flash erase for a
 * range of blocks.  The data on the card after an erase operation is
 * either 0 or 1, depends on the card vendor.  The card must support
 * single block erase.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::erase(uint32_t firstBlock, uint32_t lastBlock) {
  csd_t csd;
  if (!readCSD(&csd)) goto fail;
  // check for single block erase
  if (!csd.v1.erase_blk_en) {
    // erase size mask
    uint8_t m = (csd.v1.sector_size_high << 1) | csd.v1.sector_size_low;
    if ((firstBlock & m) != 0 || ((lastBlock + 1) & m) != 0) {
      // error card can't erase specified area
      error(SD_CARD_ERROR_ERASE_SINGLE_BLOCK);
      goto fail;
    }
  }
  if (type_ != SD_CARD_TYPE_SDHC) {
    firstBlock <<= 9;
    lastBlock <<= 9;
  }
  if (cardCommand(CMD32, firstBlock)
    || cardCommand(CMD33, lastBlock)
    || cardCommand(CMD38, 0)) {
      error(SD_CARD_ERROR_ERASE);
      goto fail;
  }
  if (!waitNotBusy(SD_ERASE_TIMEOUT)) {
    error(SD_CARD_ERROR_ERASE_TIMEOUT);
    goto fail;
  }
  chipSelectHigh();
  return true;

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** Determine if card supports single block erase.
 *
 * \return The value one, true, is returned if single block erase is supported.
 * The value zero, false, is returned if single block erase is not supported.
 */
bool Sd2Card::eraseSingleBlockEnable() {
  csd_t csd;
  return readCSD(&csd) ? csd.v1.erase_blk_en : false;
}
//------------------------------------------------------------------------------
/**
 * Initialize an SD flash memory card.
 *
 * \param[in] sckRateID SPI clock rate selector. See setSckRate().
 * \param[in] chipSelectPin SD chip select pin number.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.  The reason for failure
 * can be determined by calling errorCode() and errorData().
 */
bool Sd2Card::init(uint8_t sckRateID, uint8_t chipSelectPin) {
  errorCode_ = type_ = 0;
  chipSelectPin_ = chipSelectPin;
  // 16-bit init start time allows over a minute
  uint16_t t0 = (uint16_t)millis();
  uint32_t arg;

  pinMode(chipSelectPin_, OUTPUT);
  digitalWrite(chipSelectPin_, HIGH);
  spiBegin();

  // set SCK rate for initialization commands
  spiRate_ = SPI_SD_INIT_RATE;
  spiInit(spiRate_);

  // must supply min of 74 clock cycles with CS high.
  for (uint8_t i = 0; i < 10; i++) spiSend(0XFF);

  // command to go idle in SPI mode
  while (cardCommand(CMD0, 0) != R1_IDLE_STATE) {
    if (((uint16_t)millis() - t0) > SD_INIT_TIMEOUT) {
      error(SD_CARD_ERROR_CMD0);
      goto fail;
    }
  }
#if USE_SD_CRC
  if (cardCommand(CMD59, 1) != R1_IDLE_STATE) {
    error(SD_CARD_ERROR_CMD59);
    goto fail;
  }
#endif  // USE_SD_CRC
  // check SD version
  while (1) {
    if (cardCommand(CMD8, 0x1AA) == (R1_ILLEGAL_COMMAND | R1_IDLE_STATE)) {
      type(SD_CARD_TYPE_SD1);
      break;
    }
    for (uint8_t i = 0; i < 4; i++) status_ = spiRec();
    if (status_ == 0XAA) {
      type(SD_CARD_TYPE_SD2);
      break;
    }
    if (((uint16_t)millis() - t0) > SD_INIT_TIMEOUT) {
      error(SD_CARD_ERROR_CMD8);
      goto fail;
    }
  }
  // initialize card and send host supports SDHC if SD2
  arg = type() == SD_CARD_TYPE_SD2 ? 0X40000000 : 0;

  while (cardAcmd(ACMD41, arg) != R1_READY_STATE) {
    // check for timeout
    if (((uint16_t)millis() - t0) > SD_INIT_TIMEOUT) {
      error(SD_CARD_ERROR_ACMD41);
      goto fail;
    }
  }
  // if SD2 read OCR register to check for SDHC card
  if (type() == SD_CARD_TYPE_SD2) {
    if (cardCommand(CMD58, 0)) {
      error(SD_CARD_ERROR_CMD58);
      goto fail;
    }
    if ((spiRec() & 0XC0) == 0XC0) type(SD_CARD_TYPE_SDHC);
    // discard rest of ocr - contains allowed voltage range
    for (uint8_t i = 0; i < 3; i++) spiRec();
  }
  chipSelectHigh();
#ifndef SOFTWARE_SPI
  return setSckRate(sckRateID);
#else  // SOFTWARE_SPI
  return true;
#endif  // SOFTWARE_SPI

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/**
 * Read a 512 byte block from an SD card.
 *
 * \param[in] blockNumber Logical block to be read.
 * \param[out] dst Pointer to the location that will receive the data.

 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::readBlock(uint32_t blockNumber, uint8_t* dst) {
  SD_TRACE("RB", blockNumber);
  // use address if not SDHC card
  if (type()!= SD_CARD_TYPE_SDHC) blockNumber <<= 9;
  if (cardCommand(CMD17, blockNumber)) {
    error(SD_CARD_ERROR_CMD17);
    goto fail;
  }
  return readData(dst, 512);

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** Read one data block in a multiple block read sequence
 *
 * \param[in] dst Pointer to the location for the data to be read.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::readData(uint8_t *dst) {
  chipSelectLow();
  return readData(dst, 512);
}
//------------------------------------------------------------------------------
bool Sd2Card::readData(uint8_t* dst, size_t count) {
  uint16_t crc;
  // wait for start block token
  uint16_t t0 = millis();
  while ((status_ = spiRec()) == 0XFF) {
    if (((uint16_t)millis() - t0) > SD_READ_TIMEOUT) {
      error(SD_CARD_ERROR_READ_TIMEOUT);
      goto fail;
    }
  }
  if (status_ != DATA_START_BLOCK) {
    error(SD_CARD_ERROR_READ);
    goto fail;
  }
  // transfer data
  if (status_ = spiRec(dst, count)) {
    error(SD_CARD_ERROR_SPI_DMA);
    goto fail;
  }
  // get crc
  crc = (spiRec() << 8) | spiRec();
#if USE_SD_CRC
  if (crc != CRC_CCITT(dst, count)) {
    error(SD_CARD_ERROR_READ_CRC);
    goto fail;
  }
#endif  // USE_SD_CRC

  chipSelectHigh();
  return true;

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** read CID or CSR register */
bool Sd2Card::readRegister(uint8_t cmd, void* buf) {
  uint8_t* dst = reinterpret_cast<uint8_t*>(buf);
  if (cardCommand(cmd, 0)) {
    error(SD_CARD_ERROR_READ_REG);
    goto fail;
  }
  return readData(dst, 16);

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** Start a read multiple blocks sequence.
 *
 * \param[in] blockNumber Address of first block in sequence.
 *
 * \note This function is used with readData() and readStop() for optimized
 * multiple block reads.  SPI chipSelect must be low for the entire sequence.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::readStart(uint32_t blockNumber) {
  SD_TRACE("RS", blockNumber);
  if (type()!= SD_CARD_TYPE_SDHC) blockNumber <<= 9;
  if (cardCommand(CMD18, blockNumber)) {
    error(SD_CARD_ERROR_CMD18);
    goto fail;
  }
  chipSelectHigh();
  return true;

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** End a read multiple blocks sequence.
 *
* \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::readStop() {
  chipSelectLow();
  if (cardCommand(CMD12, 0)) {
    error(SD_CARD_ERROR_CMD12);
    goto fail;
  }
  chipSelectHigh();
  return true;

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/**
 * Set the SPI clock rate.
 *
 * \param[in] sckRateID A value in the range [0, 14].
 *
 * The SPI clock divisor will be set to approximately
 *
 * (2 + (sckRateID & 1)) << ( sckRateID/2)
 *
 * The maximum SPI rate is F_CPU/2 for \a sckRateID = 0 and the rate is
 * F_CPU/128 for \a scsRateID = 12.
 *
 * \return The value one, true, is returned for success and the value zero,
 * false, is returned for an invalid value of \a sckRateID.
 */
bool Sd2Card::setSckRate(uint8_t sckRateID) {
  if (sckRateID > MAX_SCK_RATE_ID) {
    error(SD_CARD_ERROR_SCK_RATE);
    return false;
  }
  spiRate_ = sckRateID;
  return true;
}
//------------------------------------------------------------------------------
// wait for card to go not busy
bool Sd2Card::waitNotBusy(uint16_t timeoutMillis) {
  uint16_t t0 = millis();
  while (spiRec() != 0XFF) {
    if (((uint16_t)millis() - t0) >= timeoutMillis) goto fail;
  }
  return true;

 fail:
  return false;
}
//------------------------------------------------------------------------------
/**
 * Writes a 512 byte block to an SD card.
 *
 * \param[in] blockNumber Logical block to be written.
 * \param[in] src Pointer to the location of the data to be written.
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::writeBlock(uint32_t blockNumber, const uint8_t* src) {
  SD_TRACE("WB", blockNumber);
  // use address if not SDHC card
  if (type() != SD_CARD_TYPE_SDHC) blockNumber <<= 9;
  if (cardCommand(CMD24, blockNumber)) {
    error(SD_CARD_ERROR_CMD24);
    goto fail;
  }
  if (!writeData(DATA_START_BLOCK, src)) goto fail;
  // wait for flash programming to complete
  if (!waitNotBusy(SD_WRITE_TIMEOUT)) {
    error(SD_CARD_ERROR_WRITE_TIMEOUT);
    goto fail;
  }
  // response is r2 so get and check two bytes for nonzero
  if (cardCommand(CMD13, 0) || spiRec()) {
    error(SD_CARD_ERROR_WRITE_PROGRAMMING);
    goto fail;
  }
  chipSelectHigh();
  return true;

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** Write one data block in a multiple block write sequence
 * \param[in] src Pointer to the location of the data to be written.
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::writeData(const uint8_t* src) {
  chipSelectLow();
  // wait for previous write to finish
  if (!waitNotBusy(SD_WRITE_TIMEOUT)) goto fail;
  if (!writeData(WRITE_MULTIPLE_TOKEN, src)) goto fail;
  chipSelectHigh();
  return true;

 fail:
  error(SD_CARD_ERROR_WRITE_MULTIPLE);
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
// send one block of data for write block or write multiple blocks
bool Sd2Card::writeData(uint8_t token, const uint8_t* src) {
#if USE_SD_CRC
  uint16_t crc = CRC_CCITT(src, 512);
#else  // USE_SD_CRC
  uint16_t crc = 0XFFFF;
#endif  // USE_SD_CRC

  spiSend(token);
  spiSend(src, 512);
  spiSend(crc >> 8);
  spiSend(crc & 0XFF);

  status_ = spiRec();
  if ((status_ & DATA_RES_MASK) != DATA_RES_ACCEPTED) {
    error(SD_CARD_ERROR_WRITE);
    goto fail;
  }
  return true;

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** Start a write multiple blocks sequence.
 *
 * \param[in] blockNumber Address of first block in sequence.
 * \param[in] eraseCount The number of blocks to be pre-erased.
 *
 * \note This function is used with writeData() and writeStop()
 * for optimized multiple block writes.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::writeStart(uint32_t blockNumber, uint32_t eraseCount) {
  SD_TRACE("WS", blockNumber);
  // send pre-erase count
  if (cardAcmd(ACMD23, eraseCount)) {
    error(SD_CARD_ERROR_ACMD23);
    goto fail;
  }
  // use address if not SDHC card
  if (type() != SD_CARD_TYPE_SDHC) blockNumber <<= 9;
  if (cardCommand(CMD25, blockNumber)) {
    error(SD_CARD_ERROR_CMD25);
    goto fail;
  }
  chipSelectHigh();
  return true;

 fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** End a write multiple blocks sequence.
 *
* \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool Sd2Card::writeStop() {
  chipSelectLow();
  if (!waitNotBusy(SD_WRITE_TIMEOUT)) goto fail;
  spiSend(STOP_TRAN_TOKEN);
  if (!waitNotBusy(SD_WRITE_TIMEOUT)) goto fail;
  chipSelectHigh();
  return true;

 fail:
  error(SD_CARD_ERROR_STOP_TRAN);
  chipSelectHigh();
  return false;
}
