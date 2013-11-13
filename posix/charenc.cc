#include <asn1/strings.h>
#include <asn1/machine.h>

#include <langinfo.h>
#include <iconv.h>

#include <string>
#include <stdexcept>
#include <cerrno>

using namespace asn1;

std::string
asn1::utf16_to_system (const char16_t *ptr, size_t len)
{
  char *cs = nl_langinfo (CODESET);
  iconv_t ic = iconv_open (cs, 
                           machine::is_big_endian() ? "UTF-16BE" : "UTF-16LE");
  char buffer[256];
  char *inptr = (char *)ptr;
  size_t inlen = len * 2;
  std::string result;

  if (ic == (iconv_t)-1) {
    int err = errno;
    strerror_r (err, buffer, sizeof (buffer));
    throw std::runtime_error("to_system: " + std::to_string(err) + " - " + buffer);
  }

  try {
    while (inlen) {
      char *outptr = buffer;
      size_t outlen = sizeof (buffer);
      size_t ret = iconv (ic, &inptr, &inlen, &outptr, &outlen);

      if (ret == (size_t)-1) {
        int err = errno;
        switch (err) {
        case EILSEQ:
          throw std::runtime_error("to_system: invalid UTF-16 in input");
        case EINVAL:
          throw std::runtime_error("to_system: incomplete UTF-16 in input");
        case E2BIG:
          if (!outlen)
            throw std::runtime_error("to_system: insufficient buffer space");
          break;
        default:
          strerror_r (err, buffer, sizeof (buffer));
          throw std::runtime_error("to_system: " + std::to_string(err)
                                   + " - " + buffer);
        }
      }

      result.append (buffer, outlen);
    }
  } catch (...) {
    iconv_close (ic);
    throw;
  }

  iconv_close (ic);

  return result;
}

std::u16string 
asn1::system_to_utf16(const char *ptr, size_t len)
{
  char *cs = nl_langinfo (CODESET);
  iconv_t ic = iconv_open (machine::is_big_endian() ? "UTF-16BE" : "UTF-16LE",
                           cs);
  char buffer[512];
  char *inptr = (char *)ptr;
  size_t inlen = len;
  std::u16string result;

  if (ic == (iconv_t)-1) {
    int err = errno;
    strerror_r (err, buffer, sizeof (buffer));
    throw std::runtime_error("from_system: " + std::to_string(err) 
                             + " - " + buffer);
  }

  try {
    while (inlen) {
      char *outptr = (char *)buffer;
      size_t outlen = sizeof (buffer);
      size_t ret = iconv (ic, &inptr, &inlen, &outptr, &outlen);

      if (ret == (size_t)-1) {
        int err = errno;
        switch (err) {
        case EILSEQ:
          throw std::runtime_error("from_system: invalid multibyte sequence in input");
        case EINVAL:
          throw std::runtime_error("from_system: incomplete multibyte sequence in input");
        case E2BIG:
          if (!outlen)
            throw std::runtime_error("from_system: insufficient buffer space");
          break;
        default:
          strerror_r (err, buffer, sizeof (buffer));
          throw std::runtime_error("from_system: " + std::to_string(err)
                                   + " - " + buffer);
        }
      }

      result.append ((char16_t *)buffer, outlen / 2);
    }
  } catch (...) {
    iconv_close (ic);
    throw;
  }

  iconv_close (ic);

  return result;
}

std::string
asn1::utf8_to_system (const char *ptr, size_t len)
{
  char *cs = nl_langinfo (CODESET);
  iconv_t ic = iconv_open (cs, "UTF-8");
  char buffer[256];
  char *inptr = (char *)ptr;
  size_t inlen = len;
  std::string result;

  if (ic == (iconv_t)-1) {
    int err = errno;
    strerror_r (err, buffer, sizeof (buffer));
    throw std::runtime_error("to_system: " + std::to_string(err) + " - " + buffer);
  }

  try {
    while (inlen) {
      char *outptr = buffer;
      size_t outlen = sizeof (buffer);
      size_t ret = iconv (ic, &inptr, &inlen, &outptr, &outlen);

      if (ret == (size_t)-1) {
        int err = errno;
        switch (err) {
        case EILSEQ:
          throw std::runtime_error("to_system: invalid UTF-16 in input");
        case EINVAL:
          throw std::runtime_error("to_system: incomplete UTF-16 in input");
        case E2BIG:
          if (!outlen)
            throw std::runtime_error("to_system: insufficient buffer space");
          break;
        default:
          strerror_r (err, buffer, sizeof (buffer));
          throw std::runtime_error("to_system: " + std::to_string(err)
                                   + " - " + buffer);
        }
      }

      result.append (buffer, outlen);
    }
  } catch (...) {
    iconv_close (ic);
    throw;
  }

  iconv_close (ic);

  return result;
}

std::string
asn1::system_to_utf8(const char *ptr, size_t len)
{
  char *cs = nl_langinfo (CODESET);
  iconv_t ic = iconv_open ("UTF-8", cs);
  char buffer[256];
  char *inptr = (char *)ptr;
  size_t inlen = len;
  std::string result;

  if (ic == (iconv_t)-1) {
    int err = errno;
    strerror_r (err, buffer, sizeof (buffer));
    throw std::runtime_error("from_system: " + std::to_string(err) 
                             + " - " + buffer);
  }

  try {
    while (inlen) {
      char *outptr = (char *)buffer;
      size_t outlen = sizeof (buffer);
      size_t ret = iconv (ic, &inptr, &inlen, &outptr, &outlen);

      if (ret == (size_t)-1) {
        int err = errno;
        switch (err) {
        case EILSEQ:
          throw std::runtime_error("from_system: invalid multibyte sequence in input");
        case EINVAL:
          throw std::runtime_error("from_system: incomplete multibyte sequence in input");
        case E2BIG:
          if (!outlen)
            throw std::runtime_error("from_system: insufficient buffer space");
          break;
        default:
          strerror_r (err, buffer, sizeof (buffer));
          throw std::runtime_error("from_system: " + std::to_string(err)
                                   + " - " + buffer);
        }
      }

      result.append (buffer, outlen);
    }
  } catch (...) {
    iconv_close (ic);
    throw;
  }

  iconv_close (ic);

  return result;
}

