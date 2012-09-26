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
#ifndef ArduinoStream_h
#define ArduinoStream_h
/**
 * \file
 * \brief ArduinoInStream and ArduinoOutStream classes
 */
#include <bufstream.h>
//==============================================================================
/**
 * \class ArduinoInStream
 * \brief Input stream for Arduino Stream objects
 */
class ArduinoInStream : public ibufstream {
 public:
  /**
   * Constructor
   * \param[in] hws hardware stream
   * \param[in] buf buffer for input line
   * \param[in] size size of input buffer
   */
  ArduinoInStream(Stream &hws, char* buf, uint16_t size) {
    hw_ = &hws;
    line_ = buf;
    size_ = size;
  }
  /** read a line. */
  void readline() {
    uint16_t i = 0;
    uint32_t t;
    line_[0] = '\0';
    while (!hw_->available());

    while (1) {
      t = millis();
      while (!hw_->available()) {
        if ((millis() - t) > 10) goto done;
      }
      if (i >= (size_ - 1)) {
        setstate(failbit);
        return;
      }
      line_[i++] = hw_->read();
      line_[i] = '\0';
    }
  done:
    init(line_);
  }

 protected:
  /** Internal - do not use.
   * \param[in] off
   * \param[in] way
   * \return true/false.
   */
  bool seekoff(off_type off, seekdir way) {return false;}
 /** Internal - do not use.
  * \param[in] pos
  * \return true/false.
  */
  bool seekpos(pos_type pos) {return false;}

 private:
  char *line_;
  uint16_t size_;
  Stream* hw_;
};
//==============================================================================
/**
 * \class ArduinoOutStream
 * \brief Output stream for Arduino Print objects
 */
class ArduinoOutStream : public ostream {
 public:
  /** constructor
   *
   * \param[in] pr Print object for this ArduinoOutStream.
   */
  explicit ArduinoOutStream(Print& pr) : pr_(&pr) {}

 protected:
  /// @cond SHOW_PROTECTED
  /**
   * Internal do not use
   * \param[in] c
   */
  void putch(char c) {
    if (c == '\n') pr_->write('\r');
    pr_->write(c);
  }
  void putstr(const char* str) {pr_->write(str);}
  bool seekoff(off_type off, seekdir way) {return false;}
  bool seekpos(pos_type pos) {return false;}
  bool sync() {return true;}
  pos_type tellpos() {return 0;}
  /// @endcond
 private:
  ArduinoOutStream() {}
  Print* pr_;
};
#endif  // ArduinoStream_h
