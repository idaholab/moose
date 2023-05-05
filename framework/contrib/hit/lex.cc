#include <string>
#include <vector>
#include <memory>
#include <algorithm>

#include "lex.h"
#include "braceexpr.h"

namespace hit
{

const std::string digits = "0123456789";
const std::string alpha = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const std::string space = " \t";
const std::string allspace = " \t\n\r";
const std::string newline = "\n\r";
const std::string alphanumeric = digits + alpha;
const std::string identchars = alphanumeric + "_./:<>-+*";

_LexFunc::_LexFunc(LexFunc pp) : p(pp) {}
_LexFunc::operator LexFunc() { return p; }

bool
charIn(char c, const std::string & valid)
{
  return valid.find(c) != std::string::npos;
}

// the EOF macro wreaks havok on our TokType enum which has an EOF member. So undefine it and the
// redefine it at the end of the file.
#define TMPEOF EOF
#undef EOF

int
lineCount(const std::string & input)
{
  int n = 0;
  size_t pos = input.find("\n", 0); // first occurrence
  while (pos < std::string::npos)
  {
    n++;
    pos = input.find("\n", pos + 1);
  }
  return n;
}

std::string
tokTypeName(TokType t)
{
// clang-format off
  #define tokcase(type) case TokType::type: return #type;
  switch (t)
    {
      tokcase(Error)
      tokcase(Equals)
      tokcase(LeftBracket)
      tokcase(RightBracket)
      tokcase(Ident)
      tokcase(Path)
      tokcase(Number)
      tokcase(String)
      tokcase(Comment)
      tokcase(InlineComment)
      tokcase(BlankLine)
      tokcase(EOF)
      default : return std::to_string((int)t);
    }
  #undef tokcase
  // clang-format on
}

Token::Token(TokType t,
             const std::string & val,
             const std::string & name,
             size_t offset,
             int line,
             int column)
  : type(t), val(val), name(name), offset(offset), line(line), column(column)
{
}

std::string
Token::str() const
{
  if (type == TokType::String || type == TokType::Error)
    return tokTypeName(type) + ":" + val;
  return tokTypeName(type) + ":'" + val + "'";
}

Lexer::Lexer(const std::string & name, const std::string & input) : _name(name), _input(input) {}

std::vector<Token> &
Lexer::tokens()
{
  return _tokens;
}

std::vector<Token>
Lexer::run(LexFunc start)
{
  LexFunc state = start;
  while (state != nullptr)
    state = state(this);
  return _tokens;
}

void
Lexer::rewind()
{
  if (!peek()) // don't do anything if we are at EOF
    return;

  auto tmp = lastToken();
  if (tmp >= _start)
    return;

  // subtract newlines that may have been ignored
  _line_count -= lineCount(_input.substr(tmp, _start - tmp));
  _pos = tmp;
  if (_pos < _start)
    _start = _pos;
}

void
Lexer::emit(TokType type)
{
  auto substr = _input.substr(_start, _pos - _start);
  auto column = _input.rfind('\n', _start - 1);
  if (column == std::string::npos)
    column = _start;
  else
    column = _start - column;
  _tokens.push_back(Token(type, substr, _name, _start, _line_count, column));
  _line_count += lineCount(substr);
  _start = _pos;
}

size_t
Lexer::lastToken()
{
  return _tokens.back().offset + _tokens.back().val.size();
}

void
Lexer::ignore()
{
  auto substr = _input.substr(_start, _pos - _start);
  _line_count += lineCount(substr);
  _start = _pos;
}

LexFunc
Lexer::error(const std::string & msg)
{
  _tokens.push_back(Token(TokType::Error, msg, _name, _start, _line_count));
  return nullptr;
}

char
Lexer::next()
{
  if (_pos >= _input.size())
  {
    _width = 0;
    return 0;
  }

  char c = _input[_pos];
  _width = 1;
  _pos += _width;
  return c;
}

bool
Lexer::accept(const std::string & valid)
{
  if (charIn(next(), valid))
    return true;
  backup();
  return false;
}

int
Lexer::acceptRun(const std::string & valid)
{
  int n = 0;
  while (true)
  {
    size_t index = valid.find(next());
    if (index == std::string::npos)
      break;
    n++;
  }
  backup();
  return n;
}

char
Lexer::peek()
{
  char c = next();
  backup();
  return c;
}

void
Lexer::backup()
{
  _pos = std::max(_start, _pos - _width);
}

const std::string &
Lexer::input()
{
  return _input;
}
size_t
Lexer::start()
{
  return _start;
}
size_t
Lexer::pos()
{
  return _pos;
}

_LexFunc lexHit(Lexer *);
_LexFunc lexNumber(Lexer *);
_LexFunc lexString(Lexer *);

_LexFunc
lexPath(Lexer * l)
{
  l->acceptRun(space);
  l->ignore();
  l->acceptRun(identchars);
  l->emit(TokType::Path);

  auto n = l->acceptRun(space);
  l->ignore();
  bool got_close = l->accept("]");
  if (n == 0 && !got_close)
    return l->error("invalid section path character '" + std::string(1, l->peek()) + "'");
  else if (n > 0 && !got_close)
    return l->error("spaces are not allowed in section paths");

  l->emit(TokType::RightBracket);
  return lexHit;
}

int
consumeWhitespace(Lexer * l)
{
  size_t start = 0;
  while (true)
  {
    start = l->pos();
    l->acceptRun(space);
    l->ignore();

    if (l->accept("\n"))
    {
      l->ignore();
      int n = 0;
      while (l->accept("\n"))
      {
        if (n == 0)
          l->emit(TokType::BlankLine);
        n++;
      }
    }
    if (l->pos() == start)
      break;
  }

  l->acceptRun(allspace);
  l->ignore();
  return l->pos() - start;
}

void
consumeToNewline(Lexer * l)
{
  while (true)
  {
    char c = l->next();
    if (c == '\0' || charIn(c, "\n\r"))
      break;
  }
  l->backup();
}

void
lexComments(Lexer * l)
{
  // The first comment in a file can't be an inline comment.
  if (l->start() > 0 && l->tokens().back().type != TokType::BlankLine)
  {
    l->acceptRun(space);
    l->ignore();
    if (l->accept("#"))
    {
      consumeToNewline(l);
      l->emit(TokType::InlineComment);
    }
  }

  while (true)
  {
    consumeWhitespace(l);
    if (!l->accept("#"))
      break;
    consumeToNewline(l);
    l->emit(TokType::Comment);
  }
}

_LexFunc
lexEq(Lexer * l)
{
  l->acceptRun(space);
  l->ignore();
  if (!l->accept("="))
    return l->error("expected '=' after parameter name '" + l->tokens().back().val + "', got '" +
                    std::string(1, l->next()) + "'");
  l->emit(TokType::Equals);

  l->acceptRun(allspace);
  l->ignore();

  // uncomment this to allow commentw between '=' and field value
  // lexComments(l);
  // l->acceptRun(allspace);
  // l->ignore();

  if (charIn(l->peek(), digits + "-+.eE"))
    return lexNumber;
  return lexString;
}

size_t
consumeUnquotedString(Lexer * l)
{
  while (true)
  {
    char c = l->next();
    // '#' is always a comment outside of quoted string
    if (c == '\0' || charIn(c, allspace + "[#"))
      break;
  }
  l->backup();
  return l->pos() - l->start();
}

void
consumeBraceExpression(Lexer * l)
{
  BraceNode n;
  auto offset = parseBraceNode(l->input(), l->start(), n);
  for (auto i = l->start(); i < offset; i++)
    l->next();
}

_LexFunc
lexString(Lexer * l)
{
  l->acceptRun(allspace);
  l->ignore();

  char n = l->next();
  char nn = l->peek();
  l->backup();
  if (n == '$' && nn == '{')
  {
    try
    {
      consumeBraceExpression(l);
      l->emit(TokType::String);
      return lexHit;
    }
    catch (Error & err)
    {
      return l->error(err.what());
    }
  }

  if (!charIn(l->peek(), "'\""))
  {
    if (consumeUnquotedString(l))
      l->emit(TokType::String);
    return lexHit;
  }

  std::string quote = "";
  if (l->peek() == '"')
    quote = "\"";
  else if (l->peek() == '\'')
    quote = "'";
  else
    return l->error("the parser is horribly broken");

  // this is a loop in order to enable consecutive string literals to be parsed
  while (l->accept(quote))
  {
    char c = l->input()[l->start()];
    char prev;
    while (true)
    {
      prev = c;
      c = l->next();
      if (c == quote[0] && prev != '\\')
        break;
      else if (c == '\0')
        return l->error("unterminated string");
    }
    l->emit(TokType::String);
    consumeWhitespace(l);
  }

  // unlex the last run of whitespace - we need to skip whitespace between consecutive string
  // literals, but not the final run of whitespace.  Carefully tracking/handling whitespace
  // is is important for determining how to place/format comment tokens.
  l->rewind();

  return lexHit;
}

_LexFunc
lexNumber(Lexer * l)
{
  l->accept("+-");
  int n = l->acceptRun(digits);
  if (l->accept("."))
    n += l->acceptRun(digits);

  if (l->accept("eE"))
  {
    n += l->accept("-+");
    n += l->acceptRun(digits);
  }
  if (n == 0)
  {
    // fall back to string
    if (consumeUnquotedString(l))
      l->emit(TokType::String);
    return lexHit;
  }

  if (!charIn(l->peek(), allspace + "[") && l->peek() != '\0')
  {
    // fall back to string
    if (consumeUnquotedString(l))
      l->emit(TokType::String);
    return lexHit;
  }

  l->emit(TokType::Number);
  return lexHit;
}

_LexFunc
lexHit(Lexer * l)
{
  lexComments(l);
  consumeWhitespace(l);
  char c = l->next();
  if (c == '[')
  {
    l->emit(TokType::LeftBracket);
    return lexPath;
  }
  else if (charIn(c, identchars))
  {
    l->acceptRun(identchars);
    l->emit(TokType::Ident);
    return lexEq;
  }
  else if (c == '\0')
  {
    l->emit(TokType::EOF);
    return NULL;
  }
  return l->error("invalid character '" + std::string(1, c) +
                  "' - did you leave a field value blank after a previous '='?");
}

std::vector<Token>
tokenize(const std::string & fname, const std::string & input)
{
  Lexer lex(fname, input);
  auto tokens = lex.run(lexHit);
  return tokens;
}
} // namespace hit

#define EOF TMPEOF
