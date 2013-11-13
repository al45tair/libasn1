/* Emacs, this is -*-C++-*- */

#ifndef ASN1_TIME_H_
#define ASN1_TIME_H_

#include "base.h"
#include <string>

BEGIN_ASN1_NS

enum class Basic {
  Date,
  Time,
  DateTime,
  Interval,
  RecInterval
};

enum class DateAccuracy {
  C,
  Y,
  YM,
  YMD,
  YD,
  YW,
  YWD
};

typedef int YearType_;
namespace YearType {
  enum {
    Basic,
    Proleptic,
    Negative
  };
  inline YearType_ L(int n) { return n; }
}

typedef int TimeAccuracy_;
namespace TimeAccuracy {
  enum {
    H = 0x40000000,
    M = 0x20000000,
    S = 0x10000000,
    HM = H|M,
    HMS = H|M|S,
    DIGITS = 0xfffffff
  };
  inline TimeAccuracy_ F(int n) { return n; }
  inline TimeAccuracy_ HF(int n) { return H|n; }
  inline TimeAccuracy_ HMF(int n) { return H|M|n; }
  inline TimeAccuracy_ HMSF(int n) { return H|M|S|n; }
}

enum class LocalOrUTC {
  L,
  Z,
  LD
};

enum class IntervalType {
  SE,
  D,
  SD,
  DE
};

enum class SEPoint {
  Date,
  Time,
  DateTime
};

typedef int Recurrence_;
namespace Recurrence {
  enum {
    Unlimited = 0
  };
  inline Recurrence_ R(int n) { return n; }
}

enum class Midnight {
  Start,
  End
};

class Time
{
private:
  Basic         _basic;
  DateAccuracy  _date_scale;
  YearType_     _year_type;
  TimeAccuracy_ _time_accuracy;
  LocalOrUTC    _local_or_utc;
  IntervalType  _interval_type;
  SEPoint       _se_point;
  Recurrence_   _recurrence;

  int      _year;
  unsigned _month, _week, _day;
  unsigned _hour, _minute, _second;
  unsigned _fraction;
  int      _utc_offset;
};

class Date
{
private:
  int _year;
  unsigned _month;
  unsigned _day;
};

class TimeOfDay
{
private:
  unsigned _hour;
  unsigned _minute;
  unsigned _second;
};

class DateTime
{
private:
  int      _year;
  unsigned _month;
  unsigned _day;
  unsigned _hour;
  unsigned _minute;
  unsigned _second;
};

class Duration
{
private:
  unsigned _years;
  unsigned _months;
  unsigned _weeks;
  unsigned _days;
  unsigned _hours;
  unsigned _minutes;
  unsigned _seconds;
  unsigned _fraction;

public:
  Duration() : _years(0), _months(0), _weeks(0), _days(0),
	       _hours(0), _minutes(0), _seconds(0), _fraction(0)
  {}
  Duration(const Duration &o)
    : _years(o._years), _months(o._months), _weeks(o._months), _days(o._days),
      _hours(o._hours), _minutes(o._minutes), _seconds(o._seconds),
      _fraction(o._fraction)
  {}

  Duration &operator=(const Duration &o) {
    _years = o._years; _months = o._months; _weeks = o._weeks; _days = o._days;
    _hours = o._hours; _minutes = o._minutes; _seconds = o._seconds;
    _fraction = o._fraction;
  }

  unsigned years() const { return _years; }
  unsigned months() const { return _months; }
  unsigned weeks() const { return _weeks; }
  unsigned days() const { return _days; }
  unsigned hours() const { return _hours; }
  unsigned minutes() const { return _minutes; }
  unsigned seconds() const { return _seconds; }
  unsigned fraction() const { return _fraction; }
  
  void set_years(unsigned y) { _years = y; }
  void set_months(unsigned m) { _months = m; }
  void set_weeks(unsigned w) { _weeks = w; }
  void set_days(unsigned d) { _days = d; }
  void set_hours(unsigned h) { _hours = h; }
  void set_minutes(unsigned m) { _minutes = m; }
  void set_seconds(unsigned s) { _seconds = s; }
  void set_fraction(unsigned f) { _fraction = f; }
  
  static Duration from_string(const std::string &s);
  std::string to_string() const;
};

class GeneralizedTime
{
private:
  unsigned      _year, _month, _day;
  unsigned      _hour, _minute, _second;
  unsigned      _fraction;
  TimeAccuracy_ _accuracy;
  LocalOrUTC    _local_or_utc;
  int           _utc_offset;

public:
  GeneralizedTime() : _accuracy(TimeAccuracy::HMS),
                      _year(1970), _month(1), _day(1),
                      _hour(0), _minute(0), _second(0),
                      _fraction(0), _local_or_utc(LocalOrUTC::Z),
                      _utc_offset(0)
  {}
  GeneralizedTime(unsigned y, unsigned m, unsigned d,
                  unsigned h, unsigned min, unsigned s,
                  LocalOrUTC local = LocalOrUTC::Z,
                  int utc_offset = 0)
    : _accuracy(TimeAccuracy::HMS),
      _year(y), _month(m), _day(d),
      _hour(h), _minute(min), _second(s),
      _local_or_utc(local), _utc_offset(utc_offset)
  {}
  GeneralizedTime(const GeneralizedTime &o)
    : _accuracy(o._accuracy),
      _year(o._year), _month(o._month), _day(o._day),
      _hour(o._hour), _minute(o._minute), _second(o._second),
      _local_or_utc(o._local_or_utc), _utc_offset(o._utc_offset)
  {}

  static GeneralizedTime now();
  static GeneralizedTime local_now();

  static GeneralizedTime local_from_time(time_t t);
  static GeneralizedTime from_time(time_t t);
  time_t to_time() const;

  GeneralizedTime &operator=(const GeneralizedTime &o) {
    _accuracy = o._accuracy;
    _year = o._year; _month = o._month; _day = o._day;
    _hour = o._hour; _minute = o._minute; _second = o._second;
    _fraction = o._fraction;
    _local_or_utc = o._local_or_utc;
    _utc_offset = o._utc_offset;
    return *this;
  }

  TimeAccuracy_ accuracy() const { return _accuracy; }
  unsigned year() const { return _year; }
  unsigned month() const { return _month; }
  unsigned day() const { return _day; }
  unsigned hour() const { return _hour; }
  unsigned minute() const { return _minute; }
  unsigned second() const { return _second; }
  unsigned fraction() const { return _fraction; }
  LocalOrUTC local_or_utc() const { return _local_or_utc; }
  int utc_offset() const { return _utc_offset; }

  void set_accuracy(TimeAccuracy_ a) { _accuracy = a; }
  void set_year(unsigned y) { _year = y; }
  void set_month(unsigned m) { _month = m; }
  void set_day(unsigned d) { _day = d; }
  void set_hour(unsigned h) { _hour = h; }
  void set_minute(unsigned m) { _minute = m; }
  void set_second(unsigned s) { _second = s; }
  void set_fraction(unsigned f) { _fraction = f; }
  void set_local_or_utc(LocalOrUTC lu) { _local_or_utc = lu; }
  void set_utc_offset(int o) { _utc_offset = o; }

  static GeneralizedTime from_string(const std::string &s);
  std::string to_string() const;
};

class UTCTime
{
private:
  unsigned      _year, _month, _day;
  unsigned      _hour, _minute, _second;
  TimeAccuracy_ _accuracy;
  int           _utc_offset;

public:
  UTCTime() : _accuracy(TimeAccuracy::HMS),
              _year(1970), _month(1), _day(1),
              _hour(0), _minute(0), _second(0),
              _utc_offset(0)
  {}
  UTCTime(unsigned y, unsigned m, unsigned d,
          unsigned h, unsigned min, unsigned s,
          int utc_offset = 0)
    : _accuracy(TimeAccuracy::HMS),
      _year(y), _month(m), _day(d),
      _hour(h), _minute(min), _second(s),
      _utc_offset(utc_offset)
  {}
  UTCTime(const UTCTime &o)
    : _accuracy(o._accuracy),
      _year(o._year), _month(o._month), _day(o._day),
      _hour(o._hour), _minute(o._minute), _second(o._second),
      _utc_offset(o._utc_offset)
  {
  }

  static UTCTime now();
  static UTCTime local_now();

  static UTCTime local_from_time(time_t t);
  static UTCTime from_time(time_t t);
  time_t to_time () const;

  UTCTime &operator=(const UTCTime &o) {
    _accuracy = o._accuracy;
    _year = o._year; _month = o._month; _day = o._day;
    _hour = o._hour; _minute = o._minute; _second = o._second;
    _utc_offset = o._utc_offset;
    return *this;
  }

  TimeAccuracy_ accuracy() const { return _accuracy; }
  unsigned year() const { return _year; }
  unsigned month() const { return _month; }
  unsigned day() const { return _day; }
  unsigned hour() const { return _hour; }
  unsigned minute() const { return _minute; }
  unsigned second() const { return _second; }
  int utc_offset() const { return _utc_offset; }

  void set_accuracy(TimeAccuracy_ a) { _accuracy = a; }
  void set_year(unsigned y) { _year = y; }
  void set_month(unsigned m) { _month = m; }
  void set_day(unsigned d) { _day = d; }
  void set_hour(unsigned h) { _hour = h; }
  void set_minute(unsigned m) { _minute = m; }
  void set_second(unsigned s) { _second = s; }

  static UTCTime from_string(const std::string &s);
  std::string to_string() const;
};

END_ASN1_NS

#endif /* ASN1_TIME_H_ */
