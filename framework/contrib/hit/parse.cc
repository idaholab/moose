
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <set>
#include <iterator>
#include <memory>

#include "parse.h"

// the EOF macro wreaks havok on our TokType enum which has an EOF member. So undefine it and the
// redefine it at the end of this file.
#define TMPEOF EOF
#undef EOF

namespace hit
{

const std::string indentString = "  ";

// split breaks input into a vector treating whitespace as a delimiter.  Consecutive whitespace
// characters are treated as a single delimiter.
std::vector<std::string>
split(const std::string & input)
{
  std::istringstream buffer(input);
  std::vector<std::string> ret((std::istream_iterator<std::string>(buffer)),
                               std::istream_iterator<std::string>());
  return ret;
}

// lower converts all characters in str to their lower-case versions.
std::string
lower(const std::string & str)
{
  std::string l = str;
  std::transform(l.begin(), l.end(), l.begin(), ::tolower);
  return l;
}

// trim removes consecutive whitespace characters from the beginning and end of str.
std::string
trim(const std::string & str)
{
  size_t first = str.find_first_not_of(" \r\n\t");
  if (std::string::npos == first)
    return str;
  size_t last = str.find_last_not_of(" \r\n\t");
  return str.substr(first, (last - first + 1));
}

// toBool converts the given val to a boolean value which is stored in dst.  It returns true if
// val was successfully converted to a boolean and returns false otherwise.
bool
toBool(const std::string & val, bool * dst)
{
  std::vector<std::string> trues = {"true", "on", "yes"};
  std::vector<std::string> falses = {"false", "off", "no"};
  auto s = lower(trim(val));
  for (auto & v : trues)
  {
    if (s == v)
    {
      *dst = true;
      return true;
    }
  }
  for (auto & v : falses)
  {
    if (s == v)
    {
      *dst = false;
      return true;
    }
  }
  return false;
}

std::string
nodeTypeName(NodeType t)
{
// clang-format off
  #define nodecase(type) case NodeType::type: return #type;
  switch (t)
    {
      nodecase(Root)
      nodecase(Section)
      nodecase(Comment)
      nodecase(Field)
      default : return std::to_string((int)t);
    }
  #undef nodecase
  // clang-format on
}

Error::Error(const std::string & msg) : msg(msg) {}

const char *
Error::what() const throw()
{
  return msg.c_str();
}

ParseError::ParseError(const std::string & msg) : Error(msg) {}

std::string
strRepeat(const std::string & s, int n)
{
  std::string rs;
  for (int i = 0; i < n; i++)
    rs += s;
  return rs;
}

std::string
pathNorm(const std::string & path)
{
  std::string norm;
  size_t pos = 0;
  while (pos < path.size())
  {
    if (path[pos] == '/')
    {
      while (true)
      {
        if (path[pos] == '/')
          pos++;
        else if (path.find("./", pos) == pos)
          pos += 2;
        else
          break;
      }
      norm += "/";
    }
    else
      norm += path[pos++];
  }
  if (norm.find("./", 0) == 0)
    return norm.substr(2, norm.size() - 2);
  return norm;
}

std::string
pathJoin(const std::vector<std::string> & paths)
{
  std::string fullpath;
  for (auto & p : paths)
  {
    if (p == "")
      continue;
    fullpath += "/" + p;
  }
  return fullpath.substr(1);
}

Node::Node(NodeType t) : _type(t) {}
Node::~Node()
{
  while (_children.size() > 0)
    delete _children.back();

  // remove parent's entry for this node
  if (!_parent)
    return;

  auto & parentkids = _parent->_children;
  for (auto it = parentkids.begin(); it != parentkids.end(); it++)
  {
    if (*it == this)
    {
      parentkids.erase(it);
      break;
    }
  }
}

int
Node::line()
{
  if (_toks.size() > 0)
    return _toks[0].line;
  return 0;
}

#define valthrow() throw Error("non-field node '" + fullpath() + "' has no value to retrieve")

bool
Node::boolVal()
{
  valthrow();
}
int
Node::intVal()
{
  valthrow();
}
double
Node::floatVal()
{
  valthrow();
}
std::string
Node::strVal()
{
  valthrow();
}
std::vector<double>
Node::vecFloatVal()
{
  valthrow();
}
std::vector<int>
Node::vecIntVal()
{
  valthrow();
}
std::vector<std::string>
Node::vecStrVal()
{
  valthrow();
}
#undef valthrow

NodeType
Node::type()
{
  return _type;
}

void
Node::addChild(Node * child)
{
  child->_parent = this;
  _children.push_back(child);
}

std::vector<Node *>
Node::children(NodeType t)
{
  if (t == NodeType::All)
    return _children;
  std::vector<Node *> nodes;
  for (auto child : _children)
    if (child->type() == t)
      nodes.push_back(child);
  return nodes;
}

std::string
Node::render(int indent)
{
  if (_type == NodeType::Root)
    indent = -1;

  std::string s;
  for (auto child : _children)
    s += child->render(indent + 1) + "\n";
  return s;
}

Node *
Node::parent()
{
  return _parent;
}

Node *
Node::root()
{
  if (_parent == nullptr)
    return this;
  return _parent->root();
}

std::string
Node::path()
{
  return "";
}

std::string
Node::fullpath()
{
  if (_parent == nullptr)
    return "";

  auto ppath = _parent->fullpath();
  if (ppath.empty())
    return path();
  return _parent->fullpath() + "/" + path();
}

void
Node::walk(Walker * w, NodeType t)
{
  if (_type == t || t == NodeType::All)
    w->walk(fullpath(), pathNorm(path()), this);
  for (auto child : _children)
    child->walk(w, t);
}

Node *
Node::find(const std::string & path)
{
  if (path == "" && fullpath() == "")
    return this;
  return findInner(pathNorm(path), "");
}

std::vector<Token> &
Node::tokens()
{
  return _toks;
}

Node *
Node::findInner(const std::string & path, const std::string & prefix)
{
  for (auto child : _children)
  {
    NodeType t = child->type();
    if (t != NodeType::Section && t != NodeType::Field)
      continue;

    std::string fullpath;
    if (prefix.size() == 0)
      fullpath = child->path();
    else
      fullpath = prefix + "/" + child->path();

    if (fullpath == path)
      return child;
    else if (path.find(fullpath) == std::string::npos)
      continue;

    if (child->type() == NodeType::Section)
    {
      Node * result = child->findInner(path, fullpath);
      if (result != nullptr)
        return result;
    }
  }
  return nullptr;
}

Comment::Comment(const std::string & text, bool is_inline)
  : Node(NodeType::Comment), _text(text), _isinline(is_inline)
{
}

std::string
Comment::render(int indent)
{
  if (_isinline)
    return " " + _text;
  return "\n" + strRepeat(indentString, indent) + _text;
}

Node *
Comment::clone()
{
  return new Comment(_text, _isinline);
}

Section::Section(const std::string & path) : Node(NodeType::Section), _path(pathNorm(path)) {}

std::string
Section::path()
{
  return _path;
}

std::string
Section::render(int indent)
{
  std::string s;
  if (path() != "" && tokens().size() > 4)
    s = "\n" + strRepeat(indentString, indent) + "[" + tokens()[1].val + "]";
  else if (path() != "")
    s = "\n" + strRepeat(indentString, indent) + "[" + _path + "]";

  for (auto child : children())
    if (path() == "")
      s += child->render(indent);
    else
      s += child->render(indent + 1);

  if (path() != "" && tokens().size() > 4)
    s += "\n" + strRepeat(indentString, indent) + "[" + tokens()[4].val + "]";
  else if (path() != "")
    s += "\n" + strRepeat(indentString, indent) + "[]";

  if (indent == 0 &&
      ((root() == this && s[0] == '\n') || (parent() && parent()->children()[0] == this)))
    s = s.substr(1);
  return s;
}

Node *
Section::clone()
{
  auto n = new Section(_path);
  for (auto child : children())
    n->addChild(child->clone());
  return n;
}

Field::Field(const std::string & field, Kind k, const std::string & val)
  : Node(NodeType::Field), _kind(k), _path(pathNorm(field)), _field(field), _val(val)
{
}

std::string
Field::path()
{
  return _path;
}

std::string
Field::render(int indent)
{
  std::string s = "\n" + strRepeat(indentString, indent) + _field + " = " + _val;
  for (auto child : children())
    s += child->render(indent + 1);
  return s;
}

Node *
Field::clone()
{
  return new Field(_field, _kind, _val);
}

Field::Kind
Field::kind()
{
  return _kind;
}
void
Field::setVal(const std::string & val, Kind kind)
{
  _val = val;
  if (kind != Kind::None)
    _kind = kind;
}
std::string
Field::val()
{
  return _val;
}

std::vector<int>
Field::vecIntVal()
{
  auto items = vecStrVal();
  std::vector<int> vec;
  for (auto & s : items)
  {
    try
    {
      vec.push_back(std::stoi(s));
    }
    catch (...)
    {
      throw Error("cannot convert '" + s + "' to int");
    }
  }
  return vec;
}

std::vector<double>
Field::vecFloatVal()
{
  auto items = vecStrVal();
  std::vector<double> vec;
  for (auto & s : items)
  {
    try
    {
      vec.push_back(std::stod(s));
    }
    catch (...)
    {
      throw Error("cannot convert '" + s + "' to float");
    }
  }
  return vec;
}
std::vector<std::string>
Field::vecStrVal()
{
  if (_kind != Kind::String && _kind != Kind::Int && _kind != Kind::Float)
    throw Error("field node '" + fullpath() + "' does not hold a vec-typed value (val='" + _val +
                "')");

  std::string unquoted = _val;
  if (unquoted[0] == '\'' || unquoted[0] == '"')
    unquoted = unquoted.substr(1, unquoted.size() - 2);
  return split(unquoted);
}
bool
Field::boolVal()
{
  if (_kind != Kind::Bool && _kind != Kind::Int)
    throw Error("field node '" + fullpath() + "' does not hold a bool-typed value (val='" + _val +
                "')");

  if (_kind == Kind::Int)
    return intVal();
  bool v = false;
  toBool(_val, &v);
  return v;
}
int
Field::intVal()
{
  if (_kind != Kind::Int)
    throw Error("field node '" + fullpath() + "' does not hold an int-typed value (val='" + _val +
                "')");
  try
  {
    return std::stoi(_val);
  }
  catch (...)
  {
    throw Error("cannot convert '" + _val + "' to int");
  }
}
double
Field::floatVal()
{
  if (_kind != Kind::Float && _kind != Kind::Int)
    throw Error("field node '" + fullpath() + "' does not hold a float-typed value (val='" + _val +
                "')");
  try
  {
    return std::stod(_val);
  }
  catch (...)
  {
    throw Error("cannot convert '" + _val + "' to float");
  }
}
std::string
Field::strVal()
{
  std::string s = _val;

  std::string quote = "";
  if (_val[0] == '\'')
    quote = "'";
  else if (_val[0] == '"')
    quote = "\"";

  if (quote != "")
  {
    s = _val.substr(1, _val.size() - 2);

    size_t pos = s.find("\\" + quote, 0);
    while (pos != std::string::npos)
    {
      s.replace(pos, 2, quote);
      pos += 1; // handles case where replaced text is a substring of find text
      pos = s.find("\\" + quote, pos);
    }
  }

  return s;
}

// Parser is a helper class to track progress consuming/traversing a tokenized/lexed input.
class Parser
{
public:
  Parser(const std::string & name, const std::string & input, std::vector<Token> tokens)
    : _name(name), _input(input), _tokens(tokens)
  {
  }

  size_t start() { return _start; }
  size_t pos() { return _pos; }
  std::vector<Token> & tokens() { return _tokens; }

  Token next()
  {
    if (_pos >= _tokens.size())
    {
      _pos++;
      return Token{TokType::EOF, "", _input.size()};
    }
    auto tok = _tokens[_pos];
    _pos++;
    return tok;
  }
  void backup() { _pos--; }
  Token peek()
  {
    auto tok = next();
    backup();
    return tok;
  }
  void error(Token loc, const std::string & msg)
  {
    throw ParseError(_name + ":" + std::to_string(loc.line) + ": " + msg);
  }

  Token require(TokType t, const std::string & err_msg)
  {
    auto tok = next();
    if (tok.type == TokType::Error)
      error(tok, tok.val);
    else if (tok.type != t)
      error(tok, err_msg + " (found " + tok.str() + " instead)");
    return tok;
  }
  void ignore() { _start = _pos; }
  Node * emit(Node * n)
  {
    if (n->type() == NodeType::Section)
      _current_scope.push_back(n);
    for (size_t i = _start; i < _pos; i++)
      n->tokens().push_back(_tokens[i]);
    _start = _pos;
    return n;
  }
  Node * scope()
  {
    if (_current_scope.size() == 0)
      return nullptr;
    return _current_scope.back();
  }
  void scopeclose() { _current_scope.pop_back(); }

private:
  std::vector<Node *> _current_scope;
  std::string _name;
  std::string _input;
  std::vector<Token> _tokens;
  size_t _start = 0;
  size_t _pos = 0;
};

void parseSectionBody(Parser * p, Node * n);
void parseField(Parser * p, Node * n);
void parseComment(Parser * p, Node * n);
void parseEnterPath(Parser * p, Node * n);
void parseExitPath(Parser * p, Node * n);

void
parseExitPath(Parser * p, Node * n)
{
  auto secOpenToks = p->scope()->tokens();

  auto tok = p->next();
  if (tok.type != TokType::LeftBracket)
    p->error(secOpenToks[0], "missing closing '[]' for section");

  auto path = p->require(TokType::Path, "malformed section close, expected PATH");
  p->require(TokType::RightBracket, "expected ']'");

  auto s = n->children().back();
  for (size_t i = p->start(); i < p->pos(); i++)
    s->tokens().push_back(p->tokens()[i]);

  if (path.val != "../" && path.val != "")
    p->error(path, "invalid closing path");
  p->ignore();

  p->scopeclose();
}

void
parseEnterPath(Parser * p, Node * n)
{
  p->ignore();
  p->require(TokType::LeftBracket, "");
  auto tok = p->require(TokType::Path, "invalid path in section header");
  p->require(TokType::RightBracket, "missing ']'");
  if (tok.val == "./" || tok.val == "")
    p->error(tok, "empty section name - did you mean '../'?");
  auto section = p->emit(new Section(tok.val));
  n->addChild(section);
  parseSectionBody(p, section);
  parseExitPath(p, n);
}

void
parseSectionBody(Parser * p, Node * n)
{
  while (true)
  {
    auto tok = p->next();
    auto next = p->peek();
    p->backup();
    if (tok.type == TokType::BlankLine)
    {
      p->require(TokType::BlankLine, "parser is horribly broken");
      n->addChild(p->emit(new Blank()));
    }
    else if (tok.type == TokType::Ident)
      parseField(p, n);
    else if (tok.type == TokType::Comment || tok.type == TokType::InlineComment)
      parseComment(p, n);
    else if (tok.type == TokType::LeftBracket)
    {
      if (next.type == TokType::Path && (next.val != "../" && next.val != ""))
        parseEnterPath(p, n);
      else if (p->scope() == nullptr)
        p->error(tok, "extra closing '[" + next.val + "]' found");
      else // found section closing - we are done here
        return;
    }

    else if (tok.type == TokType::Error)
      p->error(tok, tok.val);
    else if (tok.type == TokType::EOF)
      return;
    else
      p->error(tok, "unexpected token");
  }
}

void
parseField(Parser * p, Node * n)
{
  p->ignore();
  auto fieldtok = p->require(TokType::Ident, "unexpected token for field");
  p->require(TokType::Equals, "missing '='");

  Node * field = nullptr;
  auto valtok = p->next();
  if (valtok.type == TokType::Number)
  {
    std::string s = valtok.val;
    Field::Kind kind = Field::Kind::Int;
    try
    {
      if (charIn('e', s) || charIn('E', s) || charIn('.', s))
        kind = Field::Kind::Float;
      else if ((double)std::stoi(s) != std::stod(s))
        kind = Field::Kind::Float;
    }
    catch (...) // integer might be too big to fit in int - use float
    {
      kind = Field::Kind::Float;
    }

    field = p->emit(new Field(fieldtok.val, kind, s));
  }
  else if (valtok.type == TokType::String)
  {
    bool v = false;
    bool isbool = toBool(valtok.val, &v);
    if (isbool)
      field = p->emit(new Field(fieldtok.val, Field::Kind::Bool, valtok.val));
    else
      field = p->emit(new Field(fieldtok.val, Field::Kind::String, valtok.val));
  }
  else
    p->error(valtok, "malformed field value");
  n->addChild(field);
}

void
parseComment(Parser * p, Node * n)
{
  p->ignore();
  auto tok = p->next();
  bool isinline = false;
  if (tok.type == TokType::Comment)
    isinline = false;
  else if (tok.type == TokType::InlineComment)
    isinline = true;
  else
    p->error(tok, "the parser is broken");

  auto comment = p->emit(new Comment(tok.val, isinline));
  if (tok.type == TokType::InlineComment && n->children().size() > 0)
    n->children()[n->children().size() - 1]->addChild(comment);
  else
    n->addChild(comment);
}

// parse tokenizes the given hit input from fname using a Lexer with the lexHit as the
// starting LexFunc.  Parsing is implemented as recursive-descent with the functions in this file
// named "parse[Bla]".
Node *
parse(const std::string & fname, const std::string & input)
{
  Lexer lex(fname, input);
  auto tokens = lex.run(lexHit);
  Parser parser(fname, input, tokens);
  std::unique_ptr<Node> root(new Section(""));
  parseSectionBody(&parser, root.get());
  return root.release();
}

// MergeFieldWalker is used as part of the process of merging two parsed hit node trees.
class MergeFieldWalker : public Walker
{
public:
  MergeFieldWalker(Node * orig) : _orig(orig) {}
  void walk(const std::string & fullpath, const std::string & /*nodepath*/, Node * n)
  {
    auto result = _orig->find(fullpath);
    if (!result)
    {
      if (n->parent() && _orig->find(n->parent()->fullpath())) // add node to existing section
        _orig->find(n->parent()->fullpath())->addChild(n->clone());
      return;
    }
    else if (result->type() == NodeType::Field)
    {
      // node exists - overwrite its value
      auto dst = static_cast<Field *>(result);
      auto src = static_cast<Field *>(n);
      dst->setVal(src->val());
    }
  }

private:
  Node * _orig;
};

// MergeSectionWalker is used as part of the process of merging two parsed hit node trees.
class MergeSectionWalker : public Walker
{
public:
  MergeSectionWalker(Node * orig) : _orig(orig) {}
  void walk(const std::string & /*fullpath*/, const std::string & /*nodepath*/, Node * n)
  {
    auto result = _orig->find(n->fullpath());
    if (!result && n->parent())
    {
      auto anchor = _orig->find(n->parent()->fullpath());
      if (anchor)
        anchor->addChild(n->clone());
    }
  }

private:
  std::set<std::string> _done;
  Node * _orig;
};

void
merge(Node * from, Node * into)
{
  MergeFieldWalker fw(into);
  MergeSectionWalker sw(into);
  from->walk(&fw, NodeType::Field);
  from->walk(&sw, NodeType::Section);
}

Node *
explode(Node * n)
{
  if (n->type() == NodeType::Field || n->type() == NodeType::Section)
  {
    size_t pos = n->path().find("/", 0);
    if (pos != std::string::npos)
    {
      auto prefix = n->path().substr(0, pos);
      auto postfix = n->path().substr(pos + 1, n->path().size() - pos - 1);
      hit::Node * existing = nullptr;
      if (n->parent())
        existing = n->parent()->find(prefix);

      Node * newnode = nullptr;
      if (n->type() == NodeType::Field)
      {
        auto f = static_cast<Field *>(n);
        newnode = new Field(postfix, f->kind(), f->val());
      }
      else
        newnode = new Section(postfix);

      for (auto child : n->children())
        newnode->addChild(child->clone());
      newnode->tokens() = n->tokens();

      if (existing && existing->type() == NodeType::Section)
        existing->addChild(newnode);
      else
      {
        auto newsec = new Section(prefix);
        if (n->parent())
          n->parent()->addChild(newsec);
        newsec->addChild(newnode);
      }
      auto newroot = explode(newnode);
      delete n;
      return newroot->root();
    }
  }

  for (auto child : n->children())
    explode(child);
  return n->root();
}

#define EOF TMPEOF

} // namespace hit
