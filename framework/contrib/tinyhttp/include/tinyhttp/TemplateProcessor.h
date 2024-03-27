
#include <sstream>
#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <map>

struct TemplateProcessor
{
  TemplateProcessor(std::istream & ins, std::ostream & os, std::string name)
    : mInStream{ins}, mOutStream{os}, mClassName{std::move(name)}
  {
  }

  bool atEnd() const { return mInStream.eof() || mInStream.peek() == -1; }

  void process();

private:
  void handleDefault(int ch);
  void processCommand();
  void interpretPreprocessorCommand(const std::string & s);

  bool flushHtmlBuffer();
  void closeOutputLine();

  void buildConstructor();
  void buildGetSetFunctions();
  void buildMembers();

  bool mInlineFlag, mCountinousOutputFlag;

  std::istream & mInStream;
  std::ostream & mOutStream;
  std::stringstream mRenderCode;
  std::stringstream mHtmlCode;
  std::set<std::string> mHeaderIncludes;
  std::map<std::string, std::string> mParameters;
  std::string mClassName;
};
