
#include <iomanip>
#include <iterator>
#include <algorithm>
#include <regex>

#include "TemplateProcessor.h"

namespace
{
void
trim(std::string & source)
{
  source.erase(source.begin(),
               std::find_if(source.begin(),
                            source.end(),
                            [](char c) { return !isspace(static_cast<unsigned char>(c)); }));
  source.erase(std::find_if(source.rbegin(),
                            source.rend(),
                            [](char c) { return !isspace(static_cast<unsigned char>(c)); })
                   .base(),
               source.end());
}

bool
isRef(const std::string & s)
{
  return s[s.length() - 1] == '&';
}

bool
isPtr(const std::string & s)
{
  return s[s.length() - 1] == '*';
}
}

void
TemplateProcessor::process()
{
  mHeaderIncludes.insert("<HTMLTemplate.h>");

  mInlineFlag = true;
  mCountinousOutputFlag = false;

  while (!atEnd())
  {
    int c = mInStream.get();

    if (c == '<' && mInStream.peek() == '%')
    {
      mInStream.get();
      processCommand();
      continue;
    }

    handleDefault(c);
  }

  flushHtmlBuffer();
  closeOutputLine();

  mOutStream << "#ifndef _HTML_" << mClassName << "_HPP" << std::endl;
  mOutStream << "#define _HTML_" << mClassName << "_HPP" << std::endl << std::endl;

  for (const auto & c : mHeaderIncludes)
    mOutStream << "#include " << c << std::endl;

  mOutStream << "\nnamespace templates {" << std::endl;

  mOutStream << "\nstruct " << mClassName << " : HTMLTemplate {" << std::endl;

  buildConstructor();

  mOutStream << R"(
    std::string render() const override {
        std::stringstream __result;

)";
  mOutStream << mRenderCode.str();
  mOutStream << R"(
        return __result.str();
    }
)";

  buildGetSetFunctions();
  buildMembers();

  mOutStream << "};" << std::endl << "\n}\n#endif" << std::endl;
}

void
TemplateProcessor::processCommand()
{
  std::stringstream ss;

  int modifierChar = -1;

  while (true)
  {
    if (atEnd())
      throw std::runtime_error("syntax error: template command not closed");

    int ch = mInStream.get();

    if (ch == '%' && mInStream.peek() == '>')
    {
      mInStream.get();

      while (mInStream.peek() == '\n')
        mInStream.get();

      break;
    }

    if (modifierChar == -1)
    {
      if (isspace(ch))
        continue;

      modifierChar = ch;

      if (ch == '-' || ch == '=' || ch == '@')
        continue;

      modifierChar = 0;
    }

    ss << (char)ch;
  }

  std::string _s = ss.str();
  trim(_s);

  mInlineFlag = false;
  switch (modifierChar)
  {
    case '-':
      if (flushHtmlBuffer())
      {
        mRenderCode << " << escapeHTML(" << _s << ")";
        return;
      }

      mRenderCode << "        __result << escapeHTML(" << _s << ");" << std::endl;
      return;
    case '=':
      if (flushHtmlBuffer())
      {
        mRenderCode << " << " << _s;
        return;
      }

      mRenderCode << "        __result << " << _s << ";" << std::endl;
      return;
    case '@':
      interpretPreprocessorCommand(_s);
      return;
    default:
      flushHtmlBuffer();
      closeOutputLine();
      mRenderCode << "        " << _s << std::endl;
      return;
  }

  // std::co ut << "\n| command [" << _s << "], modifier [" << (char)modifierChar << "]\n\n";
}

void
TemplateProcessor::interpretPreprocessorCommand(const std::string & s)
{
  std::vector<std::string> parts;

  std::istringstream iss(s);
  std::string tmp;

  while (getline(iss, tmp, ' '))
    parts.push_back(tmp);

  if (parts[0] == "include")
  {
    std::string includeedThing = parts[1];
    trim(includeedThing);

    char ch = includeedThing[0];
    if (ch != '<' && ch != '"')
      throw std::runtime_error("Bad include syntax");

    if (ch == '"' && ch != includeedThing[includeedThing.length() - 1])
      throw std::runtime_error("Bad include syntax");

    if (ch == '<' && '>' != includeedThing[includeedThing.length() - 1])
      throw std::runtime_error("Bad include syntax");

    mHeaderIncludes.emplace(std::move(includeedThing));
    return;
  }

  if (parts[0] == "param")
  {
    mParameters.insert({parts[2], parts[1]});
    return;
  }

  // std::co ut << parts.size() << std::endl;
}

bool
TemplateProcessor::flushHtmlBuffer()
{
  if (mHtmlCode.tellp() <= 0)
    return false;

  if (!mCountinousOutputFlag)
    mRenderCode << "        __result";

  mCountinousOutputFlag = true;

  mRenderCode << " << \"" << mHtmlCode.str() << "\"";

  // reset buffer
  mHtmlCode.str("");
  mHtmlCode.clear();

  return true;
}

void
TemplateProcessor::closeOutputLine()
{
  if (mCountinousOutputFlag)
    mRenderCode << ";" << std::endl;

  mCountinousOutputFlag = false;
}

void
TemplateProcessor::buildConstructor()
{
  if (mParameters.empty())
    return;

  mOutStream << "    " << mClassName << "(";

  bool flag = true;
  for (const auto & x : mParameters)
  {
    if (!flag)
      mOutStream << ",";
    else
      flag = false;

    mOutStream << x.second << " _" << x.first;
  }

  mOutStream << ")";

  flag = true;
  mOutStream << std::endl << "        : ";

  for (const auto & x : mParameters)
  {
    if (!flag)
      mOutStream << ",";
    else
      flag = false;

    mOutStream << x.first << "(_" << x.first << ")";
  }

  mOutStream << " {}" << std::endl;
}

void
TemplateProcessor::buildGetSetFunctions()
{
  for (const auto & x : mParameters)
  {
    mOutStream << std::endl;

    std::string passType = "const " + x.second + "&";
    std::string resType = "const " + x.second + "&";

    if (isRef(x.second) || isPtr(x.second))
    {
      passType = x.second;
      resType = "const " + x.second;
    }

    mOutStream << "    " << resType << " get" << (char)std::toupper(x.first[0]) << x.first.substr(1)
               << "() const noexcept { return " << x.first << "; }\n";
    mOutStream << "    void set" << (char)std::toupper(x.first[0]) << x.first.substr(1) << "("
               << passType << " _" << x.first << ") { " << x.first << " = _" << x.first << "; }\n";
  }
}

void
TemplateProcessor::buildMembers()
{
  if (mParameters.empty())
    return;

  mOutStream << "\n    protected:" << std::endl;
  for (const auto & x : mParameters)
    mOutStream << "        " << x.second << " " << x.first << ";\n";
}

void
TemplateProcessor::handleDefault(int ch)
{
  switch (ch)
  {
    case '\r':
      mHtmlCode << "\\r";
      break;
    case '\n':
      mHtmlCode << "\\n";

      if (!mInlineFlag)
      {
        flushHtmlBuffer();
        closeOutputLine();
      }

      mInlineFlag = true;
      break;
    case '\f':
      mHtmlCode << "\\f";
      break;
    case '\t':
      mHtmlCode << "\\t";
      break;
    case '\v':
      mHtmlCode << "\\v";
      break;
    case '\0':
      mHtmlCode << "\\0";
      break;
    case '\"':
      mHtmlCode << "\\\"";
      break;
    case '\\':
      mHtmlCode << "\\\\";
      break;
    case '\a':
      mHtmlCode << "\\a";
      break;
    case '\b':
      mHtmlCode << "\\b";
      break;

    default:
      if (ch > 0x7F || ch < 0x20)
      {
        mHtmlCode << "\\x" << std::setfill('0') << std::setw(2) << std::hex << ch;
        break;
      }

      mHtmlCode << (char)ch;
      break;
  }
}
