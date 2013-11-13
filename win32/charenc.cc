#include <windows.h>

#include <asn1/strings.h>
#include <asn1/machine.h>

using namespace asn1;

std::string
asn1::utf8_to_system (const char *ptr, size_t len)
{
  std::u16string tmp = utf8_to_utf16 (ptr, len);
  return utf16_to_system (tmp.data(), tmp.length());
}

std::string
asn1::system_to_utf8 (const char *ptr, size_t len)
{
  std::u16string tmp = system_to_utf16 (ptr, len);
  return utf16_to_utf8 (tmp);
}

std::string
asn1::utf16_to_system (const char16_t *ptr, size_t len)
{
  int szlen = WideCharToMultiByte (CP_ACP,
                                   0,
                                   (LPWSTR)ptr,
                                   len,
                                   NULL,
                                   0,
                                   NULL,
                                   NULL);
  LPSTR pszTmp;
  std::string result;

  if (!szlen) {
    DWORD err = GetLastError();
    throw std::runtime_error("to_system: win32 error " + std::to_string(err));
  }

  pszTmp = (LPSTR)malloc (szlen);

  try {
    szlen = WideCharToMultiByte (CP_ACP,
                                 0,
                                 (LPCWSTR)ptr,
                                 len,
                                 pszTemp,
                                 szlen,
                                 NULL,
                                 NULL);

    if (!szlen) {
      DWORD err = GetLastError();
      throw std::runtime_error("to_system: win32 error " + std::to_string(err));
    }

    result.assign (pszTmp, szlen);
  } catch (...) {
    free (pszTmp);
    throw;
  }

  free (pszTmp);
  return result;
}

std::u16string
asn1::system_to_utf16 (const char *ptr, size_t len)
{
  int wclen = MultiByteToWideChar (CP_ACP,
                                   MB_PRECOMPOSED,
                                   ptr,
                                   len,
                                   NULL,
                                   0);
  LPWSTR pwszTmp;
  std::u16string result;

  if (!wclen) {
    DWORD err = GetLastError();
    throw std::runtime_error("from_system: win32 error " + std::to_string(err));
  }

  pwszTmp = (LPWSTR)malloc (wclen * 2);

  try {
    wclen = MultiByteToWideChar (CP_ACP,
                                 MB_PRECOMPOSED,
                                 ptr,
                                 len,
                                 pwszTmp,
                                 wclen);

    if (!wclen) {
      DWORD err = GetLastError();
      throw std::runtime_error("from_system: win32 error " + std::to_string(err));
    }

    result.assign (pwszTmp, wclen);
  } catch (...) {
    free (pwszTmp);
    throw;
  }

  free (pwszTmp);
  return result;
}
