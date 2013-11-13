#include <asn1/Time.h>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <stdexcept>

using namespace asn1;

/* .. Utilities ............................................................. */

static const unsigned days_in_month[12] = {
  31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static bool is_leap_year (int y)
{
  return y % 4 == 0 && (y % 100 || (y % 400 == 0));
}

static
unsigned scan2 (std::string::const_iterator &i)
{
  char c = *i++;
  char c2 = *i++;

  if (c < '0' || c > '9' || c2 < '0' || c2 > '9')
    throw std::runtime_error("bad time - expected two digits");

  return (c - '0') * 10 + (c2 - '0');
}

static
unsigned scan4 (std::string::const_iterator &i)
{
  char c = *i++;
  char c2 = *i++;
  char c3 = *i++;
  char c4 = *i++;

  if (c < '0' || c > '9'
      || c2 < '0' || c2 > '9'
      || c3 < '0' || c3 > '9'
      || c4 < '0' || c4 > '9')
    throw std::runtime_error("bad time - expected four digits");

  return (((c - '0') * 10 + (c2 - '0')) * 10
          + (c3 - '0')) * 10 + (c4 - '0');
}

static
unsigned scan_digits (std::string::const_iterator &i,
		      std::string::const_iterator e,
		      unsigned max = 9)
{
  unsigned dc = 0;
  unsigned r = 0;

  while (i < e) {
    char c = *i;

    if (c < '0' || c > '9')
      break;

    ++dc;
    r = r * 10 + c - '0';

    if (dc == max)
      break;
  }

  if (!dc)
    throw std::runtime_error("bad time - expected digits");

  return r;
}

static
unsigned tenpow (unsigned n)
{
  unsigned p = 10;
  unsigned r = 1;

  while (n) {
    if (n & 1)
      r *= p;
    p *= p;
    n >>= 1;
  }

  return r;
}

/* .. UTCTime .............................................................. */

UTCTime
UTCTime::now()
{
  return UTCTime::from_time_t (std::time (NULL));
}

UTCTime
UTCTime::local_now()
{
  return UTCTime::local_from_time_t (std::time (NULL));
}

UTCTime
UTCTime::from_time (time_t t)
{
  struct tm tm;
  
  std::gmtime_r (&t, &tm);

  return UTCTime (tm.tm_year + 1900,
                  tm.tm_mon + 1,
                  tm.tm_mday,
                  tm.tm_hour,
                  tm.tm_min,
                  tm.tm_sec,
                  0);
}

UTCTime
UTCTime::local_from_time (time_t t)
{
  struct tm tm;
  
  std::localtime_r (&t, &tm);

  return UTCTime (tm.tm_year + 1900,
                  tm.tm_mon + 1,
                  tm.tm_mday,
                  tm.tm_hour,
                  tm.tm_min,
                  tm.tm_sec,
                  tm.tm_gmtoff / 60);
}

time_t
UTCTime::to_time () const
{
  struct tm tm = {
    _second,
    _minute,
    _hour,
    _day,
    _month - 1,
    _year - 1900,
    0,
    0,
    0,
    NULL,
    0
  };

  return std::timegm (&tm) - 60 * _utc_offset;
}

std::string
UTCTime::to_string() const
{
  std::string r;
  char buffer[80];

  if (_accuracy & TimeAccuracy::S) {
    std::sprintf (buffer, "%02u%02u%02u%02u%02u%02u",
                  _year % 100, _month, _day,
                  _hour, _minute, _second);
  } else {
    std::sprintf (buffer, "%02u%02u%02u%02u%02u",
                  _year % 100, _month, _day,
                  _hour, _minute);
  }

  r += buffer;

  if (!_utc_offset) 
    r += 'Z';
  else {
    char s = _utc_offset < 0 ? '-' : '+';
    unsigned oa = _utc_offset < 0 ? -_utc_offset : _utc_offset;
    unsigned ho = oa / 60;
    unsigned mo = oa % 60;

    std::sprintf (buffer, "%c%02u%02u", s, ho, mo);
    r += buffer;
  }

  return r;
}

UTCTime
UTCTime::from_string (const std::string &sz)
{
  if (sz.length() < 9 || sz.length() > 15)
    throw std::runtime_error("bad UTC time");

  auto i = sz.begin();
  unsigned y, m, d;
  unsigned h, min, s;
  int offset;
  bool has_seconds;

  y = scan2(i);
  m = scan2(i);
  d = scan2(i);
  h = scan2(i);
  min = scan2(i);

  char c = *i;

  if (c == 'Z' || c == '+' || c == '-') {
    s = 0;
    has_seconds = false;
  } else {
    s = scan2(i);
    has_seconds = true;
  }

  c = *i++;

  if (c == 'Z')
    offset = 0;
  else if (c != '+' && c != '-')
    throw std::runtime_error("bad UTC time - expected '+' or '-'");
  else if (s.end() - i != 4)
    throw std::runtime_error("bad UTC time - expected four-digit tz offset");
  else {
    unsigned oh = scan2(i);
    unsigned om = scan2(i);

    if (om > 59)
      throw std::runtime_error("bad UTC time - tz offset minute out of range");

    offset = 60 * oh + om;
    if (c == '-')
      offset = -offset;
  }

  if (y < 30)
    y += 2000;
  else
    y += 1900;

  if (m < 1 || m > 12)
    throw std::runtime_error("bad UTC time - month out of range");

  if (d < 1 || d > days_in_month[month - 1]
      || (m == 2 && !is_leap_year (y) && d > 28))
    throw std::runtime_error("bad UTC time - day out of range");

  if (h > 23)
    throw std::runtime_error("bad UTC time - hour out of range");

  if (min > 59)
    throw std::runtime_error("bad UTC time - minute out of range");

  if (s > 59)
    throw std::runtime_error("bad UTC time - second out of range");

  if (i != sz.end())
    throw std::runtime_error("bad UTC time - garbage after time value");

  UTCTime u;

  u.set_accuracy (has_seconds ? TimeAccuracy::HMS : TimeAccuracy::HM);
  u.set_year (y);
  u.set_month (m);
  u.set_day (d);
  u.set_hour (h);
  u.set_minute (min);
  u.set_second (s);

  return u;
}

/* .. GeneralizedTime ....................................................... */

GeneralizedTime
GeneralizedTime::now()
{
  return GeneralizedTime::from_time_t (std::time (NULL));
}

GeneralizedTime
GeneralizedTime::local_now()
{
  return GeneralizedTime::local_from_time_t (std::time (NULL));
}

GeneralizedTime
GeneralizedTime::from_time (time_t t)
{
  struct tm tm;
  
  std::gmtime_r (&t, &tm);

  return GeneralizedTime (tm.tm_year + 1900,
                          tm.tm_mon + 1,
                          tm.tm_mday,
                          tm.tm_hour,
                          tm.tm_min,
                          tm.tm_sec,
                          LocalOrUTC::Z
                          0);
}

GeneralizedTime
GeneralizedTime::local_from_time (time_t t)
{
  struct tm tm;
  
  std::localtime_r (&t, &tm);

  return GeneralizedTime (tm.tm_year + 1900,
                          tm.tm_mon + 1,
                          tm.tm_mday,
                          tm.tm_hour,
                          tm.tm_min,
                          tm.tm_sec,
                          LocalOrUTC::LD
                          tm.tm_gmtoff / 60);
}

time_t
GeneralizedTime::to_time () const
{
  unsigned m = _minute;
  unsigned s = _second;

  // Cope with the fraction digits
  if (!(_accuracy & (TimeAccuracy::M | TimeAccuracy::S))) {
    unsigned digits = _accuracy & TimeAccuracy::DIGITS;
    unsigned frac = _fraction;

    if (digits && frac) {
      unsigned div = tenpow (digits);
      double f = 3600.0 / div;
      unsigned secs = frac * f;

      s = secs % 60;
      m = secs / 60;
    }
  } else if (!(_accuracy & TimeAccuracy::S)) {
    unsigned digits = _accuracy & TimeAccuracy::DIGITS;
    unsigned frac = _fraction;

    if (digits && frac) {
      unsigned div = tenpow (digits);
      double f = 60.0 / div;
      unsigned secs = frac * f;

      s = secs;
    }
  }

  struct tm tm = {
    s,
    m,
    _hour,
    _day,
    _month - 1,
    _year - 1900,
    0,
    0,
    0,
    NULL,
    0
  };

  return std::timegm (&tm) - 60 * _utc_offset;
}

std::string
GeneralizedTime::to_string () const
{
  std::string r;
  char buffer[80];

  sprintf(buffer, "%04u%02u%02u", _year, _month, _day);
  r = buffer;

  if (_accuracy & TimeAccuracy::S)
    sprintf (buffer, "%02u%02u%02u", _hour, _minute, _second);
  else if (_accuracy & TimeAccuracy::M)
    sprintf (buffer, "%02u%02u", _hour, _minute);
  else if (_accuracy & TimeAccuracy::H)
    sprintf (buffer, "%02u", _hour);

  r += buffer;

  unsigned digits = _accuracy && TimeAccuracy::DIGITS;
  unsigned frac = _fraction;

  // Strip trailing zeroes
  while (digits && !(frac % 10)) {
    --digits;
    frac /= 10;
  }

  if (digits) {
    sprintf (buffer, ".%0*u", digits, frac);
    r += buffer;
  }

  if (_local_or_utc == LocalOrUTC::Z || !_utc_offset)
    r += 'Z';
  else if (_local_or_utc == LocalOrUTC::LD) {
    char s = _utc_offset < 0 ? '-' : '+';
    unsigned oa = _utc_offset < 0 ? -_utc_offset : _utc_offset;
    unsigned ho = oa / 60;
    unsigned mo = oa % 60;

    std::sprintf (buffer, "%c%02u%02u", s, ho, mo);
    r += buffer;
  }

  return r;
}

GeneralizedTime
GeneralizedTime::from_string (const std::string &sz)
{
  auto e = sz.end();
  auto i = sz.begin();

  if (e - i < 8)
    throw std::runtime_error("bad GeneralizedTime - expected date");

  unsigned y, m, d;
  unsigned h, min, s;
  unsigned frac, digits;
  int offset;
  bool has_minutes, has_seconds;
  LocalOrUTC lutc;
  int utc_offset;

  y = scan4(i);
  m = scan2(i);
  d = scan2(i);

  if (e - i < 2)
    throw std::runtime_error("bad GeneralizedTime - expected time");

  h = scan2(i);

  if (i < e && (*i != '.' && *i != ',')) {
    if (e - i < 2)
      throw std::runtime_error("bad GeneralizedTime - expected minutes");
    min = scan2(i);
    has_minutes = true;

    if (i < e && (*i != '.' && *i != ',')) {
      if (e - i < 2)
        throw std::runtime_error("bad GeneralizedTime - expected seconds");
      s = scan2(i);
      has_seconds = true;
    }
  }

  digits = 0;
  frac = 0;

  if (i < e && (*i == '.' || *i == ',')) {
    ++i;

    while (i < e) {
      char ch = *i;

      if (ch < '0' || ch > '9')
        break;

      if (digits < 9) {
        ++digits;
        frac = 10 * frac + ch - '0';
      }

      ++i;
    }

    // Trim trailing zeroes
    while (digits && !(frac % 10)) {
      frac /= 10;
      --digits;
    }
  }

  lutc = L;
  utc_offset = 0;

  if (i < e) {
    if (*i == 'Z')
      lutc = Z;
    else if (*i != '+' && *i != '-')
      throw std::runtime_error("bad GeneralizedTime - expected '+' or '-'");
    else {
      char s = *i++;

      if (e - i < 4)
        throw std::runtime_error("bad GeneralizedTime - expected tz offset");

      unsigned oh = scan2(i);
      unsigned om = scan2(i);

      if (om > 59) {
        throw std::runtime_error("bad GeneralizedTime - "
                                 "tz offset minute out of range");
      }

      lutc = LD;
      utc_offset = oh * 60 + om;
      if (s == '-')
        utc_offset = -utc_offset;
    }
  }

  if (m < 1 || m > 12)
    throw std::runtime_error("bad GeneralizedTime - month out of range");

  if (d < 1 || d > days_in_month[month - 1]
      || (m == 2 && !is_leap_year (y) && d > 28))
    throw std::runtime_error("bad GeneralizedTime - day out of range");

  if (h > 23)
    throw std::runtime_error("bad GeneralizedTime - hour out of range");

  if (m > 59)
    throw std::runtime_error("bad GeneralizedTime - minute out of range");

  if (s > 59)
    throw std::runtime_error("bad GeneralizedTime - second out of range");
  
  if (i != sz.end())
    throw std::runtime_error("bad GeneralizedTime - garbage after time value");

  GeneralizedTime g;

  if (has_seconds)
    g.set_accuracy (TimeAccuracy::HMSF(digits));
  else if (has_minutes)
    g.set_accuracy (TimeAccuracy::HMF(digits));
  else
    g.set_accuracy (TimeAccuracy::HF(digits));

  g.set_year(y);
  g.set_month(m);
  g.set_day(d);
  g.set_hour(h);
  g.set_minute(min);
  g.set_second(s);
  g.set_fraction(frac);
  g.set_local_or_utc(lutc);
  g.set_utc_offset(utc_offset);

  return g;
}

/* .. Duration .............................................................. */

Duration
Duration::from_string(const std::string &sz)
{
  auto i = sz.begin(), e = sz.end();
  bool has_specifier = false;
  bool has_y = false, has_m = false, has_w = false, has_d = false;
  bool has_h = false, has_min = false, has_s = false, has_f = false;
  unsigned y, m, w, d, h, min, s, f = 0;

  if (i >= e || *i++ != 'P')
    throw std::runtime_error("bad Duration - expected duration designator");

  if (i >= e)
    throw std::runtime_error("bad Duration - at least one specifier required");

  while (i < e && *i != 'T') {
    unsigned digits = scan_digits (i, e);

    if (i >= e)
      throw std::runtime_error("bad Duration - missing time-unit designator");

    char c = *i++;

    if ((c != 'W' && has_w)
	|| (c == 'W' && has_specifier)) {
      throw std::runtime_error("bad Duration - if used, week specifiers "
			       "must be the only specifier present");
    }

    switch (c) {
    case 'W': has_w = true; w = digits; break;
    case 'Y':
      if (has_y) throw std::runtime_error("bad Duration - repeated year");
      has_y = true; y = digits; break;
    case 'M':
      if (has_m) throw std::runtime_error("bad Duration - repeated month");
      has_m = true; m = digits; break;
    case 'D':
      if (has_d) throw std::runtime_error("bad Duration - repeated day");
      has_d = true; d = digits; break;
    default:
      throw std::runtime_error("bad Duration - unrecognised designator");
    }

    has_specifier = true;
  }

  if (*i == 'T') {
    ++i;
    while (i < e && !has_f) {
      unsigned digits = scan_digits (i, e);

      if (i >= e)
	throw std::runtime_error("bad Duration - missing time-unit designator");

      char c = *i++;

      if (c == ',' || c == '.') {
	f = scan_digits (i, e);
	has_f = true;
      }

      switch (c) {
      case 'H':
	if (has_h) throw std::runtime_error("bad Duration - repeated hour");
	has_h = true; h = digits; break;
      case 'M': 
	if (has_min) throw std::runtime_error("bad Duration - repeated minute");
	has_min = true; min = digits; break;
      case 'S': 
	if (has_s) throw std::runtime_error("bad Duration - repeated second");
	has_s = true; s = digits; break;
      default:
	throw std::runtime_error("bad Duration - unrecognised designator");
      }
    }
  }

  if (i != e)
    throw std::runtime_error("bad Duration - garbage after end");

  
}
