#include "fakeString.h"
#include <string>
#include <stdlib.h>

String::String(const char *cstr)
  : length(strlen(cstr))
{
  strlcpy(this->cstr, cstr, length + 1);
}

String::String(const long val)
  : cstr(""),
    length(0)
{
}

String::String(const String &rval)
  : length(rval.length)
{
  strlcpy(cstr, rval.cstr, rval.length + 1);
}

String::String(String &rval)
  : length(rval.length)
{
  strlcpy(cstr, rval.cstr, rval.length + 1);
}

String & String::operator = (const String &rhs)
{
  length = rhs.length;
  strlcpy(cstr, rhs.cstr, length + 1);
	return *this;
}

String & String::operator = (const char *cstr)
{
  length = strlen(cstr);
  strlcpy(this->cstr, cstr, length + 1);
	return *this;
}

const char* String::c_str() const {
    return this->cstr;
}

char String::charAt(unsigned int index) const {
    return this->cstr[index];
}

int String::indexOf(char c) const {
  const char *pch = strchr(cstr, c);
  return pch - cstr;
}

int String::indexOf(char ch, unsigned int fromIndex) const {
  const char *pch = strchr(cstr + fromIndex, ch);
  return pch - cstr;
}

String String::substring( unsigned int beginIndex, unsigned int endIndex ) const {
  char substr[1024];
  strlcpy(substr, cstr + beginIndex, endIndex - beginIndex + 1);

  String newString(substr);
	return newString;
}

double String::toFloat() const {
    return atof(cstr);
}

int String::toInt() const {
    return atoi(cstr);
}
