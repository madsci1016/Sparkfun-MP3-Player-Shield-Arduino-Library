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
#ifndef bufstream_h
#define bufstream_h
/**
 * \file
 * \brief \ref ibufstream and \ref obufstream classes
 */
#include <iostream.h>
//==============================================================================
/**
 * \class ibufstream
 * \brief parse a char string
 */
class ibufstream : public istream {
 public:
  /** Constructor */
  ibufstream() : buf_(0), len_(0) {}
  /** Constructor
   * \param[in] str pointer to string to be parsed
   * Warning: The string will not be copied so must stay in scope.
   */
  explicit ibufstream(const char* str) {
  init(str);
  }
  /** Initialize an ibufstream
   * \param[in] str pointer to string to be parsed
   * Warning: The string will not be copied so must stay in scope.
   */
  void init(const char* str) {
    buf_ = str;
    len_ = strlen(buf_);
    pos_ = 0;
    clear();
  }

 protected:
  /// @cond SHOW_PROTECTED
  int16_t getch() {
    if (pos_ < len_) return buf_[pos_++];
    setstate(eofbit);
    return -1;
  }
  void getpos(fpos_t *pos) {
    pos->position = pos_;
  }
  bool seekoff(off_type off, seekdir way) {return false;}
  bool seekpos(pos_type pos) {
    if (pos < len_) {
      pos_ = pos;
      return true;
    }
    return false;
  }
  void setpos(fpos_t *pos) {
    pos_ = pos->position;
  }
  pos_type tellpos() {
    return pos_;
  }
  /// @endcond
 private:
  const char* buf_;
  uint16_t len_;
  uint16_t pos_;
};
//==============================================================================
/**
 * \class obufstream
 * \brief format a char string
 */
class obufstream : public ostream {
 public:
  /** constructor */
  obufstream() : in_(0) {}
  /** Constructor
   * \param[in] buf buffer for formatted string
   * \param[in] size buffer size
   */
  obufstream(char *buf, uint16_t size) {
    init(buf, size);
  }
  /** Initialize an obufstream
   * \param[in] buf buffer for formatted string
   * \param[in] size buffer size
   */
  void init(char *buf, uint16_t size) {
    buf_ = buf;
    buf[0] = '\0';
    size_ = size;
    in_ = 0;
  }
  /** \return a pointer to the buffer */
  char* buf() {return buf_;}
  /** \return the length of the formatted string */
  uint16_t length() {return in_;}

 protected:
  /// @cond SHOW_PROTECTED
  void putch(char c) {
    if (in_ >= (size_ - 1)) {
      setstate(badbit);
      return;
    }
    buf_[in_++] = c;
    buf_[in_]= '\0';
  }
  void putstr(const char *str) {
    while (*str) putch(*str++);
  }
  bool seekoff(off_type off, seekdir way) {return false;}
  bool seekpos(pos_type pos) {
    if (pos > in_) return false;
    in_ = pos;
    buf_[in_] = '\0';
    return true;
  }
  bool sync() {return true;}

  pos_type tellpos() {
    return in_;
  }
  /// @endcond
 private:
  char *buf_;
  uint16_t size_;
  uint16_t in_;
};
#endif  // bufstream_h
