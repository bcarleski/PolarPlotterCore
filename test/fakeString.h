#ifndef _FAKE_STRING_
#define _FAKE_STRING_

class String
{
private:
    char cstr[1024];
    int length;

public:
    String(const char *cstr = "");
    String(const long val);
    String(const String &rval);
    String(String &rval);

	String & operator = (const String &rhs);
	String & operator = (const char *cstr);
	String & operator + (const String &rhs)	{return (*this);}
	String & operator + (const char *cstr)		{return (*this);}
	String & operator + (char c)			{return (*this);}
	String & operator + (unsigned char num)		{return (*this);}
	String & operator + (int num)			{return (*this);}
	String & operator + (unsigned int num)		{return (*this);}
	String & operator + (long num)			{return (*this);}
	String & operator + (unsigned long num)	{return (*this);}
	String & operator + (float num)		{return (*this);}
	String & operator + (double num)		{return (*this);}

    const char* c_str() const;
    char charAt(unsigned int index) const;
    int indexOf(char c) const;
    String substring( unsigned int beginIndex ) const { return substring(beginIndex, this->length); };
	String substring( unsigned int beginIndex, unsigned int endIndex ) const;
    float toFloat() const;
    int toInt() const;
};
#endif
