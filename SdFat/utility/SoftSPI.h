#ifndef SoftSPI_h
#define SoftSPI_h
#include <DigitalPin.h>

/** nop for timing */
#define nop asm volatile ("nop\n\t")
//------------------------------------------------------------------------------
/**
 * \class SoftSPI
 * \brief fast bit-bang SPI
 */
template<uint8_t MisoPin, uint8_t MosiPin, uint8_t SckPin, uint8_t Mode = 0>
class SoftSPI {
 public:
 //-----------------------------------------------------------------------------
 /** initialize SoftSpi */
  void begin() {
    fastPinMode(MisoPin, false);
    fastPinMode(MosiPin, true);
    fastPinMode(SckPin, true);
    fastDigitalWrite(SckPin, MODE_CPOL(Mode));
  }
  //----------------------------------------------------------------------------
  /** Soft SPI receive byte
   * \return byte received
   */
  uint8_t receive() {
    uint8_t data = 0;
    receiveBit(7, &data);
    receiveBit(6, &data);
    receiveBit(5, &data);
    receiveBit(4, &data);
    receiveBit(3, &data);
    receiveBit(2, &data);
    receiveBit(1, &data);
    receiveBit(0, &data);
    return data;
  }
  //----------------------------------------------------------------------------
  /** Soft SPI send byte
   * \param[in] data byte to send
   */
  void send(uint8_t data) {
    sendBit(7, data);
    sendBit(6, data);
    sendBit(5, data);
    sendBit(4, data);
    sendBit(3, data);
    sendBit(2, data);
    sendBit(1, data);
    sendBit(0, data);
  }
  //----------------------------------------------------------------------------
  /** Soft SPI transfer byte
   * \param[in] txData byte to send
   * \return byte received
   */
  inline __attribute__((always_inline))
  uint8_t transfer(uint8_t txData) {
    uint8_t rxData = 0;
    transferBit(7, &rxData, txData);
    transferBit(6, &rxData, txData);
    transferBit(5, &rxData, txData);
    transferBit(4, &rxData, txData);
    transferBit(3, &rxData, txData);
    transferBit(2, &rxData, txData);
    transferBit(1, &rxData, txData);
    transferBit(0, &rxData, txData);
    return rxData;
  }
 private:
  //----------------------------------------------------------------------------
  inline __attribute__((always_inline))
  bool MODE_CPHA(uint8_t mode) {return (mode & 1) != 0;}
  inline __attribute__((always_inline))
  bool MODE_CPOL(uint8_t mode) {return (mode & 2) != 0;}
  inline __attribute__((always_inline))
  void receiveBit(uint8_t bit, uint8_t* data) {
    if (MODE_CPHA(Mode)) {
      fastDigitalWrite(SckPin, !MODE_CPOL(Mode));
    }
    nop;nop;
    fastDigitalWrite(SckPin,
      MODE_CPHA(Mode) ? MODE_CPOL(Mode) : !MODE_CPOL(Mode));
    if (fastDigitalRead(MisoPin)) *data |= 1 << bit;
    if (!MODE_CPHA(Mode)) {
      fastDigitalWrite(SckPin, MODE_CPOL(Mode));
    }
  }
  //----------------------------------------------------------------------------
  inline __attribute__((always_inline))
  void sendBit(uint8_t bit, uint8_t data) {
    if (MODE_CPHA(Mode)) {
      fastDigitalWrite(SckPin, !MODE_CPOL(Mode));
    }
    fastDigitalWrite(MosiPin, data & (1 << bit));
    fastDigitalWrite(SckPin,
      MODE_CPHA(Mode) ? MODE_CPOL(Mode) : !MODE_CPOL(Mode));
    nop;nop;
    if (!MODE_CPHA(Mode)) {
      fastDigitalWrite(SckPin, MODE_CPOL(Mode));
    }
  }
  //----------------------------------------------------------------------------
  inline __attribute__((always_inline))
  void transferBit(uint8_t bit, uint8_t* rxData, uint8_t txData) {
    if (MODE_CPHA(Mode)) {
      fastDigitalWrite(SckPin, !MODE_CPOL(Mode));
    }
    fastDigitalWrite(MosiPin, txData & (1 << bit));
    fastDigitalWrite(SckPin,
      MODE_CPHA(Mode) ? MODE_CPOL(Mode) : !MODE_CPOL(Mode));
    if (fastDigitalRead(MisoPin)) *rxData |= 1 << bit;
    if (!MODE_CPHA(Mode)) {
      fastDigitalWrite(SckPin, MODE_CPOL(Mode));
    }
  }
  //----------------------------------------------------------------------------
};
#endif  // SoftSPI_h