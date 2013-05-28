#ifndef __STOUT_BYTES_HPP__
#define __STOUT_BYTES_HPP__

#include <ctype.h> // For 'isdigit'.
#include <stdint.h>

#include <iomanip>
#include <iostream>
#include <string>

#include "numify.hpp"
#include "strings.hpp"
#include "try.hpp"


class Bytes
{
public:
  static Try<Bytes> parse(const std::string& s)
  {
    size_t index = 0;

    while (index < s.size()) {
      if (isdigit(s[index])) {
        index++;
        continue;
      } else if (s[index] == '.') {
        return Error("Fractional bytes '" + s + "'");
      }

      Try<uint64_t> value = numify<uint64_t>(s.substr(0, index));

      if (value.isError()) {
        return Error(value.error());
      }

      const std::string& unit = strings::upper(s.substr(index));

      if (unit == "B") {
        return Bytes(value.get(), BYTES);
      } else if (unit == "KB") {
        return Bytes(value.get(), KILOBYTES);
      } else if (unit == "MB") {
        return Bytes(value.get(), MEGABYTES);
      } else if (unit == "GB") {
        return Bytes(value.get(), GIGABYTES);
      } else if (unit == "TB") {
        return Bytes(value.get(), TERABYTES);
      } else {
        return Error("Unknown bytes unit '" + unit + "'");
      }
    }
    return Error("Invalid bytes '" + s + "'");
  }

  Bytes(uint64_t bytes = 0) : value(bytes) {}
  Bytes(uint64_t _value, uint64_t _unit) : value(_value * _unit) {}

  // TODO(bmahler): Consider killing kilobytes to terabyte helpers, given
  // they implicitly lose precision if not careful.
  uint64_t bytes()     const { return value; }
  uint64_t kilobytes() const { return value / KILOBYTES; }
  uint64_t megabytes() const { return value / MEGABYTES; }
  uint64_t gigabytes() const { return value / GIGABYTES; }
  uint64_t terabytes() const { return value / TERABYTES; }

  bool operator <  (const Bytes& that) const { return value <  that.value; }
  bool operator <= (const Bytes& that) const { return value <= that.value; }
  bool operator >  (const Bytes& that) const { return value >  that.value; }
  bool operator >= (const Bytes& that) const { return value >= that.value; }
  bool operator == (const Bytes& that) const { return value == that.value; }
  bool operator != (const Bytes& that) const { return value != that.value; }

  Bytes& operator += (const Bytes& that)
  {
    value += that.value;
    return *this;
  }

  Bytes& operator -= (const Bytes& that)
  {
    value -= that.value;
    return *this;
  }

protected:
  static const uint64_t BYTES = 1;
  static const uint64_t KILOBYTES = 1024 * BYTES;
  static const uint64_t MEGABYTES = 1024 * KILOBYTES;
  static const uint64_t GIGABYTES = 1024 * MEGABYTES;
  static const uint64_t TERABYTES = 1024 * GIGABYTES;

private:
  uint64_t value;
};


class Kilobytes : public Bytes
{
public:
  explicit Kilobytes(uint64_t value) : Bytes(value, KILOBYTES) {}
};


class Megabytes : public Bytes
{
public:
  explicit Megabytes(uint64_t value) : Bytes(value, MEGABYTES) {}
};


class Gigabytes : public Bytes
{
public:
  explicit Gigabytes(uint64_t value) : Bytes(value, GIGABYTES) {}
};


class Terabytes : public Bytes
{
public:
  explicit Terabytes(uint64_t value) : Bytes(value, TERABYTES) {}
};


inline std::ostream& operator << (std::ostream& stream, const Bytes& bytes)
{
  // Only raise the unit when there is no loss of information.
  if (bytes.bytes() == 0) {
    return stream << bytes.bytes() << "B";
  } else if (bytes.bytes() % 1024 != 0) {
    return stream << bytes.bytes() << "B";
  } else if (bytes.kilobytes() % 1024 != 0) {
    return stream << bytes.kilobytes() << "KB";
  } else if (bytes.megabytes() % 1024 != 0) {
    return stream << bytes.megabytes() << "MB";
  } else if (bytes.gigabytes() % 1024 != 0) {
    return stream << bytes.gigabytes() << "GB";
  } else {
    return stream << bytes.terabytes() << "TB";
  }
}


inline Bytes operator + (const Bytes& lhs, const Bytes& rhs)
{
  Bytes sum = lhs;
  sum += rhs;
  return sum;
}


inline Bytes operator - (const Bytes& lhs, const Bytes& rhs)
{
  Bytes diff = lhs;
  diff -= rhs;
  return diff;
}

#endif // __STOUT_BYTES_HPP__
