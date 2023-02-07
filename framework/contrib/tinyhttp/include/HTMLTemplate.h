
#include <string>
#include <sstream>

#ifndef _HTML_TEMPLATE_H
#define _HTML_TEMPLATE_H

template <typename T>
inline T
escapeHTML(const T & what)
{
  return what;
}

template <>
inline std::string
escapeHTML<std::string>(const std::string & what)
{
  std::string res;
  res.reserve(what.size());

  for (const auto & ch : what)
  {
    switch (ch)
    {
      case '&':
        res.append("&amp;");
        break;
      case '\"':
        res.append("&quot;");
        break;
      case '\'':
        res.append("&apos;");
        break;
      case '<':
        res.append("&lt;");
        break;
      case '>':
        res.append("&gt;");
        break;
      default:
        res.append(&ch, 1);
        break;
    }
  }

  return res;
}

struct HTMLTemplate
{
  virtual std::string render() const = 0;
};

#endif
