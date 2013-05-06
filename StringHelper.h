#ifndef STRING_HELPER_H_2006_03_28
#define STRING_HELPER_H_2006_03_28

#include <cassert>
#include <typeinfo>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include "CommonMacro.h"

//  please include this file in cpp file, if you want to use SplitUTF8/UTF16to8/UTF8to16
//#include "utfcpp/include/utf8.h"

BEGIN_NAMESPACE(NSVirgo)
BEGIN_NAMESPACE(NSStringHelper)

/////////////////////////////////////////////////////////////////////////////////

/** @namespace StringHelper
 *  @brief Collection of functions for processing strings.
 *
 *  The namespace consists of functions for processing strings. <br>
 *  Most of these functions is applicable to both std::string and std::wstring. <br>
 *  The range definition is consistent with STL, i.e. [Begin, End) .
 *
 *  @author Yun Huang, huangyunict@gmail.com
 *  @date 2006-11-07
 *
 */

typedef std::string::size_type size_type;   /**< typedef size_type. */

/** @name Char Traits 
 *
 *  Traits in this group provide traits extractor for char_type and string_type.
 *
 */ 
/** @{ */
/** @class CharTraits
 *  @brief The traits extractor for char_type and string_type.
 *
 *  @author Yun Huang, huangyunict@gmail.com
 *  @date 2010-11-27
 *
 */
template <typename T>
struct CharTraits
{
    typedef typename T::char_type char_type;       /**< The char type. */
    typedef typename T::string_type string_type;   /**< The string type. */
};

/** @class CharTraits.
 *  @brief Template specification for std::string.
 *
 *  @author Yun Huang, huangyunict@gmail.com
 *  @date 2010-11-27
 */
template <>
struct CharTraits<std::string>          /**< Template specification for std::string. */
{
    typedef char char_type;             /**< The char type. */
    typedef std::string string_type;    /**< The string type. */
};

/** @class CharTraits.
 *  @brief Template specification for std::wstring.
 *
 *  @author Yun Huang, huangyunict@gmail.com
 *  @date 2010-11-27
 */
template <>
struct CharTraits<std::wstring>         /**< Template specification for std::wstring. */
{
    typedef wchar_t char_type;          /**< The char type. */
    typedef std::wstring string_type;   /**< The string type. */
};
/** @} */

/////////////////////////////////////////////////////////////////////////////////

BEGIN_NAMESPACE(detail)

template <typename TChar, typename TValue>
std::basic_ostream<TChar>& WriteToStream(std::basic_ostream<TChar>& OS, const TValue& value)
{
    OS << value;
    return OS;
}

//  FOut: ostream& operator << (ostream& OS, const iterator_traits<TInputIter>::value_type& x)
template <typename TChar, typename TInputIter, typename FWrite>
std::basic_string<TChar> Concat(TInputIter begin, TInputIter end, const std::basic_string<TChar>& sep, FWrite fWrite)
{
    std::basic_ostringstream<TChar> oss;
    for (TInputIter it=begin; it!=end; ++it)
    {
        if (it != begin)
        {
            oss << sep;
        }
        fWrite(oss, *it);
    }
    return oss.str();
}

template <typename TChar>
std::basic_string<TChar> Replace(const std::basic_string<TChar>& str, const std::basic_string<TChar>& old_str, const std::basic_string<TChar>& new_str, size_type* pReplaced)
{
    if (old_str.empty())
    {
        return str;
    }
    std::basic_string<TChar> s;
    if (pReplaced)
    {
        *pReplaced = 0;
    }
    size_type prevpos = 0, pos;
    while (true)
    {
        pos = str.find(old_str, prevpos);
        if (pos == std::basic_string<TChar>::npos)
        {
            pos = str.size();
        }
        s.insert(s.size(), str, prevpos, pos - prevpos);
        if (pos == str.size())
        {
            break;
        }
        s.insert(s.size(), new_str);
        if (pReplaced)
        {
            ++(*pReplaced);
        }
        prevpos = pos + old_str.size();
    }
    return s;
}

template <typename TChar>
inline std::basic_string<TChar> ReplaceStart(const std::basic_string<TChar>& str, const std::basic_string<TChar>& old_str, const std::basic_string<TChar>& new_str, size_type* pReplaced)
{
    //  TODO: reimplement, optimize
    if (pReplaced)
    {
        *pReplaced = 0;
    }
    if (old_str.empty())
    {
        return str;
    }
    std::basic_string<TChar> s = str;
    if (s.find(old_str) == 0)
    {
        s.replace(0, old_str.size(), new_str);
        if (pReplaced)
        {
            ++(*pReplaced);
        }
    }
    return s;
}

template <typename TChar>
inline std::basic_string<TChar> ReplaceEnd(const std::basic_string<TChar>& str, const std::basic_string<TChar>& old_str, const std::basic_string<TChar>& new_str, size_type* pReplaced)
{
    if (pReplaced)
    {
        *pReplaced = 0;
    }
    //  TODO: reimplement, optimize
    if (old_str.empty())
    {
        return str;
    }
    std::basic_string<TChar> s = str;
    size_type pos = s.rfind(old_str);
    if (pos != std::basic_string<TChar>::npos && pos + old_str.size() == s.size())
    {
        s.replace(pos, old_str.size(), new_str);
        if (pReplaced)
        {
            ++(*pReplaced);
        }
    }
    return s;
}

END_NAMESPACE(detail)

/////////////////////////////////////////////////////////////////////////////////

/** @name CConv/Widen/Narrow Functions 
 *
 *  Functions in this group convert between char and wchar_t.
 *  @note Only available for ASCII characters or strings. <br>
 *        If narrow failed to convert, return '\\0'.
 */
/** @{ */
/** @brief Convert between C++ type char and C++ type wchar_t.
 *
 *  @param[in] AChar The input character.
 *  @return Return the converted character.
 *  @note If failed to convert, return '\\0'.
 */
template <typename TOutputCharT, typename TInputCharT>
TOutputCharT CConv(TInputCharT AChar);

/** @brief Convert between std::string and std::wstring.
 *
 *  @param[in] str The input string or wstring.
 *  @return Return the converted string or wstring.
 */
template <typename TOutputCharT, typename TInputCharT>
std::basic_string<TOutputCharT> CConv(const std::basic_string<TInputCharT>& str);

/** @brief Widen C++ type char to given output type.
 *
 *  @param[in] AChar The input character.
 *  @return Return the converted character.
 */
template <typename TChar>
TChar Widen(char AChar);

/** @brief Widen C-style string to std::basic_string of given type.
 *
 *  @param[in] str The input string.
 *  @return Return the converted string.
 */
template <typename TChar>
std::basic_string<TChar> Widen(const char* str);

/** @brief Widen std::string to std::basic_string of given type.
 *
 *  @param[in] str The input string.
 *  @return Return the converted string.
 */
template <typename TChar>
std::basic_string<TChar> Widen(const std::string& str);

/** @brief Narrow C++ type wchar_t to given output type.
 *
 *  @param[in] AChar The input character.
 *  @return Return the converted character.
 */
template <typename TChar>
TChar Narrow(wchar_t AChar);

/** @brief Narrow C-style wstring to std::basic_string of given type.
 *
 *  @param[in] str The input string.
 *  @return Return the converted string.
 */
template <typename TChar>
std::basic_string<TChar> Narrow(const wchar_t* str);

/** @brief Narrow std::wstring to std::basic_string of given type.
 *
 *  @param[in] str The input string.
 *  @return Return the converted string.
 */
template <typename TChar>
std::basic_string<TChar> Narrow(const std::wstring& str);
/** @} */


/** @name Convert Functions 
 *
 *  Functions in this group convert between string and other types.
 *  @note These function do not check whether actually converted.
 */
/** @{ */
/** @brief Convert string to other type.
 *
 *  @param[in] str The input string.
 *  @return Return converted value of other type.
 */
template <typename TOther>
TOther String2Other(const std::string& str);

template <typename TOther>
TOther& String2Other(const std::string& str, TOther& other);

/** @brief Convert string to other type. Template specification for std::string.
 *
 *  @param[in] str The input string.
 *  @return Return converted value of other type.
 */
template <>
std::string String2Other<std::string>(const std::string& str);

template <>
std::string& String2Other<std::string>(const std::string& str, std::string& other);

/** @brief Convert other type to string.
 *
 *  @param[in] other The input value of other type.
 *  @param[in] prec The precision used in string stream.
 *  @return Return converted string.
 *  @note If input is a string, return the input, do NOT use string stream, i.e. spaces are kept.
 */
template <typename TOther>
std::string Other2String(const TOther& AOther, std::streamsize prec = 6);
/** @} */


/** @name Concatenate/Split Functions 
 *
 *  Functions in this group concatenate multiple item to string, or split string into multiple items.
 */
/** @{ */
/** @brief Concatenate every item in range to string, separated by given separator. 
 *
 *  @param[in] begin The begin iterator.
 *  @param[in] end The end iterator.
 *  @param[in] sep The separator string written between two items.
 *  @return Return the concatenated string.
 */
template <typename TInputIter>
std::string Concat(TInputIter begin, TInputIter end, const std::string& sep = " ");
template <typename TInputIter>
std::wstring ConcatW(TInputIter begin, TInputIter end, const std::wstring& sep = L" ");
template <typename TInputIter, typename FSelect>
std::string Concat(TInputIter begin, TInputIter end, const std::string& sep, FSelect fSelect);

/** @brief Pad some characters on the right of given string to get new string with given length.
 *
 *  @param[in] str The input string.
 *  @param[in] size The new string size.
 *  @param[in] c The padding character.
 *  @return Return the padded string with given length.
 *  @note If the length of input string is greater than the given size, assert failed.
 */
std::string PadRight(const std::string& str, size_type size, char c = ' ');

/** @brief Split items from stream separated by spaces.
 *
 *  The items of given element type are separated by space characters, and output to iterator.
 *
 *  @param[in] IS The input stream.
 *  @param[in] result The output iterator.
 *  @return Return the output iterator pointed to next output position.
 */
template <typename TElem, typename TOutputIter>
TOutputIter SplitSpace(std::istream& IS, TOutputIter result);

/** @brief Split items from string separated by spaces.
 *
 *  The items of given element type are separated by space characters, and output to iterator.
 *  @param[in] str The input string.
 *  @param[in] result The output iterator.
 *  @return Return the output iterator pointed to next output position.
 */
template <typename TElem, typename TOutputIter>
TOutputIter SplitSpace(const std::string& str, TOutputIter result);

/** @brief Split items from string separated by given character.
 *
 *  @param[in] str The input string.
 *  @param[in] result The output iterator.
 *  @param[in] sep The separating character.
 *  @param[in] output_empty Whether to output, if there is nothing between two adjacent separators.
 *  @return Return the output iterator pointed to next output position.
 */
template <typename TElem, typename TOutputIter>
TOutputIter SplitChar(const std::string& str, TOutputIter result, char sep = ' ', bool output_empty = false);

/** @brief Split items from string separated by given string.
 *
 *  @param[in] str The input string.
 *  @param[in] result The output iterator.
 *  @param[in] sep The separating string.
 *  @param[in] output_empty Whether to output, if there is nothing between two adjacent separators.
 *  @return Return the output iterator pointed to next output position.
 */
template <typename TElem, typename TOutputIter>
TOutputIter SplitString(const std::string& str, TOutputIter result, const std::string& sep = " ", bool output_empty = false);

//  TODO:
/** @brief Split Chinese characters from string in GB18030 encoding.
 *
 *  @param[in] str The input string.
 *  @param[in] result The output iterator.
 *  @return Return the output iterator pointed to next output position.
 *  @note Chinese character is two bytes, one byte followed by negative char.
 *  @exception Throw error message.
 */
template <typename TOutputIter>
TOutputIter SplitGB(const std::string& str, TOutputIter result);

#ifdef  NSVIRGO_NSUTF8
template <typename TOutputIter>
TOutputIter SplitUTF8(const std::string& str, TOutputIter result);

std::string UTF16to8(const std::wstring& str);

std::wstring UTF8to16(const std::string& str);

#endif
/** @} */


/** @name Case change Functions 
 *
 *  Functions in this group could change the lower/upper case of char or string.
 *  @note If change std::string, do NOT change the byte following negative byte. <br>
 *        If change std::wstring, do NOT check whether negative.
 */
/** @{ */
/** @brief Change case of single character.
 *
 *  @param[in] AChar The character.
 *  @param[in] AU2L  If true, convert from upper to lower, otherwise from lower to upper. 
 *  @return If converted, return the converted character, otherwise return the input character. 
 */
template <typename TChar>
static TChar ChangeCase(TChar AChar, bool AU2L);

/** @brief Change case of range.
 *
 *  @param[in] ABegin The begin iterator.
 *  @param[in] AEnd   The end iterator.
 *  @param[in] AU2L  If true, convert from upper to lower, otherwise from lower to upper. 
 */
template <typename TChar, typename TForwardIter>
static void ChangeCase(TForwardIter ABegin, TForwardIter AEnd, bool AU2L);

/** @brief Convert single character to lower case.
 *
 *  @param[in]  AChar The character.
 *  @return If converted, return the converted character, otherwise return the input   character.
 */
template <typename TChar>
TChar ToLower(TChar AChar);

/** @brief Convert string to lower case.
 *
 *  @param[in]  str The input string.
 *  @return Return the converted string.
 */
template <typename TChar>
std::basic_string<TChar> ToLower(const std::basic_string<TChar>& str);

/** @brief Convert range to lower case in place.
 *
 *  @param[in] ABegin The begin iterator.
 *  @param[in] AEnd   The end iterator.
 */
template <typename TChar, typename TForwardIter>
void ToLowerInplace(TForwardIter ABegin, TForwardIter AEnd);

/** @brief Convert string to lower case in place.
 *
 *  @param[in]  str The input string.
 *  @param[out] str The output string.
 */
template <typename TChar>
void ToLowerInplace(std::basic_string<TChar>& str);

/** @brief Convert single character to upper case.
 *
 *  @param[in]  AChar The character.
 *  @return If converted, return the converted character, otherwise return the input   character.
 */
template <typename TChar>
TChar ToUpper(TChar AChar);

/** @brief Convert string to upper case.
 *
 *  @param[in]  str The input string.
 *  @return Return the converted string.
 */
template <typename TChar>
std::basic_string<TChar> ToUpper(const std::basic_string<TChar>& str);

/** @brief Convert range to upper case in place.
 *
 *  @param[in] ABegin The begin iterator.
 *  @param[in] AEnd   The end iterator.
 */
template <typename TChar, typename TForwardIter>
void ToUpperInplace(TForwardIter ABegin, TForwardIter AEnd);

/** @brief Convert string to upper case in place.
 *
 *  @param[in]  str The input string.
 *  @param[out] str The output string.
 */
template <typename TChar>
void ToUpperInplace(std::basic_string<TChar>& str);
/** @} */

/** @name Trim Functions 
 *
 *  Functions in this group could trim left/right characters of string.
 */
/** @{ */

//  unique
template <typename TChar>
std::basic_string<TChar>& UniqueInplace(std::basic_string<TChar>& str, const TChar& c = Widen<TChar>(' '));


/** @brief Trim all given characters in left of string.
 *
 *  @param[in]  str The input string.
 *  @param[in]  ATrim   The character set to trim.
 *  @param[out] str The output string after trimming.
 *  @return Return the reference of trimmed string.
 */
template <typename TChar>
std::basic_string<TChar>& TrimLeft(std::basic_string<TChar>& str, const std::basic_string<TChar>& ATrim = Widen<TChar>(std::string(" \r\n\t")));

/** @brief Trim all given characters in right of string.
 *
 *  @param[in]  str The input string.
 *  @param[in]  ATrim   The character set to trim.
 *  @param[out] str The output string after trimming.
 *  @return Return the reference of trimmed string.
 */
template <typename TChar>
std::basic_string<TChar>& TrimRight(std::basic_string<TChar>& str, const std::basic_string<TChar>& ATrim = Widen<TChar>(std::string(" \r\n\t")));

/** @brief Trim all given characters in both left and right of string.
 *
 *  @param[in]  str The input string.
 *  @param[in]  ATrim   The character set to trim.
 *  @param[out] str The output string after trimming.
 *  @return Return the reference of trimmed string.
 */
template <typename TChar>
std::basic_string<TChar>& Trim(std::basic_string<TChar>& str, const std::basic_string<TChar>& ATrim = Widen<TChar>(std::string(" \r\n\t")));
/** @} */


/** @name Find/Replace Functions
 *
 *  Find/Replace functions.
 */
/** @{ */

template <typename TChar>
bool StartsWith(const std::basic_string<TChar>& str, const std::basic_string<TChar>& sub);

template <typename TChar>
bool EndsWith(const std::basic_string<TChar>& str, const std::basic_string<TChar>& sub);

/** @brief Replace old sub string to new sub string.
 *
 *  @param[in]  str The input string.
 *  @param[in]  old_str The old sub string.
 *  @param[in]  new_str The new sub string.
 *  @param[out] pReplaced The number of replacing.
 *  @return Return replaced string.
 */
template <typename TChar>
std::basic_string<TChar> Replace(const std::basic_string<TChar>& str, const std::basic_string<TChar>& old_str, const std::basic_string<TChar>& new_str, size_type* pReplaced = NULL);

template <typename TChar>
std::basic_string<TChar> ReplaceStart(const std::basic_string<TChar>& str, const std::basic_string<TChar>& old_str, const std::basic_string<TChar>& new_str, size_type* pReplaced = NULL);

template <typename TChar>
std::basic_string<TChar> ReplaceEnd(const std::basic_string<TChar>& str, const std::basic_string<TChar>& old_str, const std::basic_string<TChar>& new_str, size_type* pReplaced = NULL);

template <typename TChar>
std::basic_string<TChar>& ReplaceInplace(std::basic_string<TChar>& str, const std::basic_string<TChar>& old_str, const std::basic_string<TChar>& new_str, size_type* pReplaced = NULL);

template <typename TChar>
std::basic_string<TChar>& ReplaceStartInplace(std::basic_string<TChar>& str, const std::basic_string<TChar>& old_str, const std::basic_string<TChar>& new_str, size_type* pReplaced = NULL);

template <typename TChar>
std::basic_string<TChar>& ReplaceEndInplace(std::basic_string<TChar>& str, const std::basic_string<TChar>& old_str, const std::basic_string<TChar>& new_str, size_type* pReplaced = NULL);
/** @} */


/** @name Miscellaneous Functions
 *
 *  Miscellaneous functions.
 */
/** @{ */

/** @brief Get content between the begin and end XML tags.
 *
 *  @param[in]  str The input string.
 *  @param[in]  tag The XML tag.
 *  @return Return the content between the begin and end XML tags. 
 *  @note Get content from "<foo id="1"\>bar</foo\>" given tag "foo" will be "bar".
 *  @exception If format error, throw exception. 
 */
std::string XMLContent(const std::string& str, const std::string& tag);

/** @brief Get the value of given attribute.
 *
 *  @param[in]  str The input string.
 *  @param[in]  tag The XML tag.
 *  @param[in]  attribute The given attribute name.
 *  @return Return the value of given attribute. 
 *  @note Get value from "<foo id="1"\>bar</foo\>" given tag "foo" and attribute "id" will be "1".
 *  @exception If format error, throw exception. 
 */
std::string XMLAttribute(const std::string& str, const std::string& tag, const std::string& attribute);
/** @} */

END_NAMESPACE(NSStringHelper)
END_NAMESPACE(NSVirgo)

////////////////////////////////////////////////////////////////////////////////
//  implementation

BEGIN_NAMESPACE(NSVirgo)
BEGIN_NAMESPACE(NSStringHelper)

/** @brief Instantiate CConv.
 */
template <>
inline char CConv<char, char>(char AChar)
{
    return AChar;
}

/** @brief Instantiate CConv.
 */
template <>
inline wchar_t CConv<wchar_t, char>(char AChar)
{
    return std::wcout.widen(AChar);
}

/** @brief Instantiate CConv.
 */
template <>
inline char CConv<char, wchar_t>(wchar_t AChar)
{
    return std::cout.narrow(AChar, '\0');
}

/** @brief Instantiate CConv.
 */
template <>
inline wchar_t CConv<wchar_t, wchar_t>(wchar_t AChar)
{
    return AChar;
}

/** @brief Instantiate CConv.
 */
template <>
inline std::basic_string<char> CConv<char, char>(const std::string& str)
{
    return str;
}

/** @brief Instantiate CConv.
 */
template <>
inline std::basic_string<wchar_t> CConv<wchar_t, char>(const std::string& str)
{
    std::basic_string<wchar_t> ws;
    ws.resize(str.size());
    for (size_type i=0; i<str.size(); ++i)
    {
        ws[i] = CConv<wchar_t>(str[i]);
    }
    return ws;
}

/** @brief Instantiate CConv.
 */
template <>
inline std::basic_string<char> CConv<char, wchar_t>(const std::wstring& str)
{
    std::basic_string<char> s;
    s.resize(str.size());
    for (size_type i=0; i<str.size(); ++i)
    {
        s[i] = CConv<char>(str[i]);
    }
    return s;
}

/** @brief Instantiate CConv.
 */
template <>
inline std::basic_string<wchar_t> CConv<wchar_t, wchar_t>(const std::wstring& str)
{
    return str;
}

template <typename TChar>
inline TChar Widen(char AChar)
{
    return CConv<TChar>(AChar);
}

template <typename TChar>
inline std::basic_string<TChar> Widen(const char* str)
{
    return Widen<TChar>(std::string(str));
}

template <typename TChar>
std::basic_string<TChar> Widen(const std::string& str)
{
    return CConv<TChar>(str);
}

template <typename TChar>
inline TChar Narrow(wchar_t AChar)
{
    return CConv<TChar>(AChar);
}

template <typename TChar>
inline std::basic_string<TChar> Narrow(const wchar_t* str)
{
    return Narrow<TChar>(std::string(str));
}

template <typename TChar>
inline std::basic_string<TChar> Narrow(const std::wstring& str)
{
    return CConv<TChar>(str);
}

template <typename TInputIter>
inline std::string Concat(TInputIter begin, TInputIter end, const std::string& sep)
{
    return detail::Concat(begin, end, sep, detail::WriteToStream<char, typename std::iterator_traits<TInputIter>::value_type>);
}

template <typename TInputIter>
inline std::wstring ConcatW(TInputIter begin, TInputIter end, const std::wstring& sep)
{
    return detail::Concat(begin, end, sep, detail::WriteToStream<wchar_t, typename std::iterator_traits<TInputIter>::value_type>);
}

template <typename TInputIter, typename FSelect>
inline std::string Concat(TInputIter begin, TInputIter end, const std::string& sep, FSelect fSelect)
{
    return detail::Concat(begin, end, sep, bind(detail::WriteToStream<char, typename std::iterator_traits<TInputIter>::value_type>(std::placeholders::_1, fSelect(std::placeholders::_2))));
}

inline std::string PadRight(const std::string& str, size_type size, char c)
{
    assert(str.size() <= size);
    return str + std::string(size - str.size(), c);
}

template <typename TChar>
inline TChar ChangeCase(TChar AChar, bool AU2L)
{
    TChar srcb, srce, tgtb;
    //  set variables
    if (AU2L)
    {
        srcb = TChar('A');
        tgtb = TChar('a');
    }
    else
    {
        srcb = TChar('a');
        tgtb = TChar('A');
    }
    srce = srcb + ('Z' - 'A');
    //  change case 
    if (AChar >= srcb && AChar <= srce)
    {
        AChar = AChar - srcb + tgtb;
    }
    return AChar;
}

template <typename TChar, typename TForwardIter>
void ChangeCase(TForwardIter ABegin, TForwardIter AEnd, bool AU2L)
{
    bool charflag;
    if (typeid(TChar) == typeid(char))
    {
        charflag = true;
    }
    else
    {
        charflag = false;
    }
    bool flag = false;  //  only used in string
    for (; ABegin!=AEnd; ++ABegin)
    {
        if (charflag && flag)
        {
            flag = false;
        }
        else if (charflag && *ABegin < 0)
        {
            flag = true;
        }
        else
        {
            TChar c = static_cast<TChar>(*ABegin);
            *ABegin = ChangeCase<TChar>(c, AU2L);
        }
    }
}

template <typename TOther>
inline TOther String2Other(const std::string& str)
{
    //std::cerr<<"in general String2Other: TChar="<<typeid(TChar).name()<<" TOther="<<typeid(TOther).name()<<std::endl;
    TOther other;
    String2Other(str, other);
    return other;
}

template <typename TOther>
inline TOther& String2Other(const std::string& str, TOther& other)
{
    //std::cerr<<"in general String2Other: TChar="<<typeid(TChar).name()<<" TOther="<<typeid(TOther).name()<<std::endl;
    //  TODO: delete the dynamic check of typeid, use static check
    assert(typeid(TOther) != typeid(std::string));
    assert(typeid(TOther) != typeid(std::wstring));
    std::istringstream iss(str);
    iss >> other;
    if (iss.fail())
    {
        throw std::runtime_error("String2Other: convert failed for string: " + str);
    }
    return other;
}

template <>
inline std::string String2Other<std::string>(const std::string& str)
{
    //std::cerr<<"in char String2Other"<<std::endl;
    return str;
}

template <>
inline std::string& String2Other<std::string>(const std::string& str, std::string& other)
{
    //std::cerr<<"in char String2Other"<<std::endl;
    if (&str != &other)
    {
        other = str;
    }
    return other;
}

template <typename TOther>
inline std::string Other2String(const TOther& other, std::streamsize prec)
{
    std::ostringstream oss;
    oss.precision(prec);
    oss << other;
    return oss.str();
}

template <typename TChar>
inline bool StartsWith(const std::basic_string<TChar>& str, const std::basic_string<TChar>& sub)
{
    if (str.size() < sub.size())
    {
        return false;
    }
    return std::equal(sub.begin(), sub.end(), str.begin());
}

template <typename TChar>
inline bool EndsWith(const std::basic_string<TChar>& str, const std::basic_string<TChar>& sub)
{
    if (str.size() < sub.size())
    {
        return false;
    }
    return std::equal(sub.begin(), sub.end(), str.begin() + str.size() - sub.size()); 
}

template <typename TChar>
inline std::basic_string<TChar> Replace(const std::basic_string<TChar>& str, const std::basic_string<TChar>& old_str, const std::basic_string<TChar>& new_str, size_type* pReplaced)
{
    return detail::Replace<TChar>(str, old_str, new_str, pReplaced);
}

template <typename TChar>
inline std::basic_string<TChar> ReplaceStart(const std::basic_string<TChar>& str, const std::basic_string<TChar>& old_str, const std::basic_string<TChar>& new_str, size_type* pReplaced)
{
    return detail::ReplaceStart(str, old_str, new_str, pReplaced);
}

template <typename TChar>
inline std::basic_string<TChar> ReplaceEnd(const std::basic_string<TChar>& str, const std::basic_string<TChar>& old_str, const std::basic_string<TChar>& new_str, size_type* pReplaced)
{
    return detail::ReplaceEnd(str, old_str, new_str, pReplaced);
}

template <typename TChar>
inline std::basic_string<TChar>& ReplaceInplace(std::basic_string<TChar>& str, const std::basic_string<TChar>& old_str, const std::basic_string<TChar>& new_str, size_type* pReplaced)
{
    str = detail::Replace(str, old_str, new_str, pReplaced);
    return str;
}

template <typename TChar>
inline std::basic_string<TChar>& ReplaceStartInplace(std::basic_string<TChar>& str, const std::basic_string<TChar>& old_str, const std::basic_string<TChar>& new_str, size_type* pReplaced)
{
    str = detail::ReplaceStart(str, old_str, new_str, pReplaced);
    return str;
}

template <typename TChar>
inline std::basic_string<TChar>& ReplaceEndInplace(std::basic_string<TChar>& str, const std::basic_string<TChar>& old_str, const std::basic_string<TChar>& new_str, size_type* pReplaced)
{
    str = detail::ReplaceEnd(str, old_str, new_str, pReplaced);
    return str;
}

template <typename TElem, typename TOutputIter>
TOutputIter SplitSpace(std::istream& IS, TOutputIter result)
{
    TElem elem;
    while (IS >> elem)
    {
        *result = elem;
        ++result;
    }
    return result;
}

template <typename TElem, typename TOutputIter>
inline TOutputIter SplitSpace(const std::string& str, TOutputIter result)
{
    std::istringstream iss;
    iss.str(str);
    return SplitSpace<TElem, TOutputIter>(iss, result);
}

template <typename TElem, typename TOutputIter>
inline TOutputIter SplitChar(const std::string& str, TOutputIter result, char sep, bool output_empty)
{
    std::string s(1, sep);
    return SplitString<TElem, TOutputIter>(str, result, s, output_empty);
}

template <typename TElem, typename TOutputIter>
TOutputIter SplitString(const std::string& str, TOutputIter result, const std::string& sep, bool output_empty)
{
    size_type pos1 = 0;
    size_type pos2 = 0;
    if (str.empty() || sep.empty())
    {
        return result;
    }
    while (true)
    {
        pos2 = str.find(sep, pos1);
        if (pos2 == std::string::npos)
        {
            pos2 = str.size();
        }
        if (pos2 != pos1 || output_empty)
        {
            *result = String2Other<TElem>(str.substr(pos1, pos2 - pos1));
            ++result;
        }
        pos1 = pos2 + sep.size();
        if (pos2 == str.size())
        {
            break;
        }
    }
    return result;
}

template <typename TOutputIter>
TOutputIter SplitGB(const std::string& str, TOutputIter result)
{
    bool flag = false;
    std::string word;
    for (size_type i=0; i<str.size(); ++i)
    {
        char c = str[i];
        if (flag)
        {
            word += c;
            *result++ = word;
            word = "";
            flag = false;
        }
        else
        {
            if (c < 0)
            {
                flag = true;
                assert(word.empty());
                word += c;
            }
            else
            {
                assert(word.empty());
                word += c;
                *result++ = word;
                word = "";
            }
        }
    }
    if (flag)
    {
        throw std::runtime_error("string ends with a negative character.");
    }
    return result;
}

#ifdef  NSVIRGO_NSUTF8

template <typename TOutputIter>
TOutputIter SplitUTF8(const std::string& str, TOutputIter result)
{
    for (auto it=str.begin(); it<str.end(); )
    {
        char symbol[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};   //  actually 5 zeros should be enough 
        uint32_t code = utf8::next(it, str.end());
        if (code == 0)
        {
            continue;
        }
        utf8::append(code, symbol); // initialize array `symbol`
        (*result++) = std::string(symbol);
    }
    return result;
}

inline std::string UTF16to8(const std::wstring& str)
{
    std::string utf_str;
    utf8::utf16to8(str.begin(), str.end(), back_inserter(utf_str));
    return utf_str;
}

inline std::wstring UTF8to16(const std::string& str)
{
    std::wstring utf_str;
    utf8::utf8to16(str.begin(), str.end(), back_inserter(utf_str));
    return utf_str;
}

#endif

template <typename TChar>
inline TChar ToLower(TChar AChar)
{
    return ChangeCase<TChar>(AChar, true);
}

template <typename TChar>
inline std::basic_string<TChar> ToLower(const std::basic_string<TChar>& str)
{
    std::basic_string<TChar> s = str;
    ToLowerInplace(s);
    return s;
}

template <typename TChar, typename TForwardIter>
inline void ToLowerInplace(TForwardIter ABegin, TForwardIter AEnd)
{
    ChangeCase<TChar>(ABegin, AEnd, true);
}

template <typename TChar>
inline void ToLowerInplace(std::basic_string<TChar>& str)
{
    ChangeCase<TChar>(str.begin(), str.end(), true);
}

template <typename TChar>
inline TChar ToUpper(TChar AChar)
{
    return ChangeCase<TChar>(AChar, false);
}

template <typename TChar>
inline std::basic_string<TChar> ToUpper(const std::basic_string<TChar>& str)
{
    std::basic_string<TChar> s = str;
    ToUpperInplace(s);
    return s;
}

template <typename TChar, typename TForwardIter>
inline void ToUpperInplace(TForwardIter ABegin, TForwardIter AEnd)
{
    ChangeCase<TChar>(ABegin, AEnd, false);
}

template <typename TChar>
inline void ToUpperInplace(std::basic_string<TChar>& str)
{
    ChangeCase<TChar>(str.begin(), str.end(), false);
}

template <typename TChar>
inline std::basic_string<TChar>& UniqueInplace(std::basic_string<TChar>& str, const TChar& c)
{
    str.erase(std::unique(str.begin(), str.end(), [&](const TChar& x, const TChar& y) -> bool { return (x == c) && (y == c); }), str.end());
    return str;
}

template <typename TChar>
inline std::basic_string<TChar>& TrimLeft(std::basic_string<TChar>& str, const std::basic_string<TChar>& ATrim)
{
    if (!str.empty())
    {
        size_type pos = str.find_first_not_of(ATrim, 0);
        if (pos == std::basic_string<TChar>::npos)
        {
            pos = str.size();
        }
        str.erase(0, pos);
    }
    return str;
}

template <typename TChar>
inline std::basic_string<TChar>& TrimRight(std::basic_string<TChar>& str, const std::basic_string<TChar>& ATrim)
{
    if (!str.empty())
    {
        size_type pos = str.find_last_not_of(ATrim, str.size());
        if (pos != std::basic_string<TChar>::npos)
        {
            str.erase(pos + 1, str.size() - pos);
        }
        else
        {
            str.erase(0, str.size());
        }
    }
    return str;
}

template <typename TChar>
inline std::basic_string<TChar>& Trim(std::basic_string<TChar>& str, const std::basic_string<TChar>& ATrim)
{
    TrimLeft(str, ATrim);
    TrimRight(str, ATrim);
    return str;
}

inline std::string XMLContent(const std::string& str, const std::string& tag)
{
    std::string start_tag = std::string("<") + tag;
    std::string end_tag = std::string("</") + tag + std::string(">");
    //  start pos
    if (str.find(start_tag) != 0)
    {
        std::string msg = std::string("XMLContent: start_tag \"") + tag + std::string("\" is not in the beginning.");
        throw std::runtime_error(msg);
    }
    size_type start_pos = str.find('>', 0);
    if (start_pos == std::string::npos)
    {
        std::string msg = std::string("XMLContent: can not find \'>\' for start_tag \"") + tag + std::string("\".");
        throw std::runtime_error(msg);
    }
    ++start_pos;
    //  end pos
    size_type end_pos = str.find(end_tag, start_pos);
    if (end_pos == std::string::npos)
    {
        std::string msg = std::string("XMLContent: can not find end_tag \"") + tag + std::string("\"");
        throw std::runtime_error(msg);
    }
    //  content
    return str.substr(start_pos, end_pos - start_pos);
}

inline std::string XMLAttribute(const std::string& str, const std::string& tag, const std::string& attribute)
{
    std::string start_tag = std::string("<") + tag;
    std::string id_tag = attribute + std::string("=");
    //  start pos
    if (str.find(start_tag) != 0)
    {
        std::string msg = std::string("XMLAttribute: start_tag \"") + tag + std::string("\" is not in the beginning.");
        throw std::runtime_error(msg);
    }
    size_type end_pos = str.find('>', 0);
    if (end_pos == std::string::npos)
    {
        std::string msg = std::string("XMLAttribute: can not find \'>\' for start_tag \"") + tag + std::string("\".");
        throw std::runtime_error(msg);
    }
    //  id pos
    size_type attribute_start_pos = str.find(id_tag, start_tag.size());
    if (attribute_start_pos == std::string::npos || attribute_start_pos > end_pos)
    {
        std::string msg = std::string("XMLAttribute: can not find attribute \"") + attribute + std::string("\".");
        throw std::runtime_error(msg);
    }
    attribute_start_pos += id_tag.size();
    bool quote = false;
    if (str[attribute_start_pos] == '\"')
    {
        quote = true;
        ++attribute_start_pos;
    }
    size_type attribute_end_pos;
    if (quote)
    {
        attribute_end_pos = str.find_first_of("\">", attribute_start_pos);
    }
    else
    {
        attribute_end_pos = str.find_first_of(" >", attribute_start_pos);
    }
    if (attribute_end_pos == std::string::npos || attribute_end_pos > end_pos)
    {
        std::string msg = std::string("XMLAttribute: can not find \'\"\' for attribute \"") + attribute + std::string("\".");
        throw std::runtime_error(msg);
    }
    if (quote)
    {
        if (str[attribute_end_pos] != '\"')
        {
            std::string msg = std::string("XMLAttribute: missing right quote for attribute \"") + attribute + std::string("\".");
            throw std::runtime_error(msg);
        }
    }
    else
    {
        if (str[attribute_end_pos] == '\"')
        {
            std::string msg = std::string("XMLAttribute: missing left quote do not match for attribute \"") + attribute + std::string("\".");
            throw std::runtime_error(msg);
        }
    }
    return str.substr(attribute_start_pos, attribute_end_pos - attribute_start_pos);
}

/////////////////////////////////////////////////////////////////////////////////

END_NAMESPACE(NSStringHelper)
END_NAMESPACE(NSVirgo)

#endif

