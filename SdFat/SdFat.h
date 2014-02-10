/* Arduino SdFat Library
 * Copyright (C) 2012 by William Greiman
 *
 * This file is part of the Arduino SdFat Library
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
 * along with the Arduino SdFat Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef SdFat_h
#define SdFat_h
/**
 * \file
 * \brief SdFat class
 */
//------------------------------------------------------------------------------
/** SdFat version YYYYMMDD */
#define SD_FAT_VERSION 20130629
//------------------------------------------------------------------------------
/** error if old IDE */
#if !defined(ARDUINO) || ARDUINO < 100
#error Arduino IDE must be 1.0 or greater
#endif  // ARDUINO < 100
//------------------------------------------------------------------------------
#include <SdFile.h>
#include <SdStream.h>
#include <ArduinoStream.h>
#include <MinimumSerial.h>
//------------------------------------------------------------------------------
/**
 * \class SdFat
 * \brief Integration class for the %SdFat library.
 */
class SdFat {
 public:
  SdFat() {}
#if ALLOW_DEPRECATED_FUNCTIONS && !defined(DOXYGEN)
  /**
   * Initialize an SdFat object.
   *
   * Initializes the SD card, SD volume, and root directory.
   *
   * \param[in] sckRateID value for SPI SCK rate. See Sd2Card::init().
   * \param[in] chipSelectPin SD chip select pin. See Sd2Card::init().
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   */
  bool init(uint8_t sckRateID = SPI_FULL_SPEED,
    uint8_t chipSelectPin = SD_CHIP_SELECT_PIN) {
    return begin(chipSelectPin, sckRateID);
  }
#elif  !defined(DOXYGEN)  // ALLOW_DEPRECATED_FUNCTIONS
  bool init() __attribute__((error("use sd.begin()")));
  bool init(uint8_t sckRateID)
    __attribute__((error("use sd.begin(chipSelect, sckRate)")));
  bool init(uint8_t sckRateID, uint8_t chipSelectPin)
    __attribute__((error("use sd.begin(chipSelect, sckRate)")));
#endif  // ALLOW_DEPRECATED_FUNCTIONS
  /** \return a pointer to the Sd2Card object. */
  Sd2Card* card() {return &card_;}
  bool chdir(bool set_cwd = false);
  bool chdir(const char* path, bool set_cwd = false);
  void chvol();
  void errorHalt();
  void errorHalt(char const *msg);
  void errorPrint();
  void errorPrint(char const *msg);
  bool exists(const char* name);
  bool begin(uint8_t chipSelectPin = SD_CHIP_SELECT_PIN,
    uint8_t sckRateID = SPI_FULL_SPEED);
  void initErrorHalt();
  void initErrorHalt(char const *msg);
  void initErrorPrint();
  void initErrorPrint(char const *msg);
  void ls(uint8_t flags = 0);
  void ls(Print* pr, uint8_t flags = 0);
  bool mkdir(const char* path, bool pFlag = true);
  bool remove(const char* path);
  bool rename(const char *oldPath, const char *newPath);
  bool rmdir(const char* path);
  bool truncate(const char* path, uint32_t length);
  /** \return a pointer to the SdVolume object. */
  SdVolume* vol() {return &vol_;}
  /** \return a pointer to the volume working directory. */
  SdBaseFile* vwd() {return &vwd_;}
  //----------------------------------------------------------------------------
  void errorHalt_P(PGM_P msg);
  void errorPrint_P(PGM_P msg);
  void initErrorHalt_P(PGM_P msg);
  void initErrorPrint_P(PGM_P msg);
  //----------------------------------------------------------------------------
  /**
   *  Set stdOut Print stream for messages.
   * \param[in] stream The new Print stream.
   */
  static void setStdOut(Print* stream) {stdOut_ = stream;}
  /** \return Print stream for messages. */
  static Print* stdOut() {return stdOut_;}

 private:
  Sd2Card card_;
  SdVolume vol_;
  SdBaseFile vwd_;
  static Print* stdOut_;
};
#endif  // SdFat_h
