#include <algorithm>
#include <iterator>
#include <memory>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "parse.h"

namespace hit
{

std::string
quoteChar(const std::string & s)
{
  if (s[0] == '\'')
    return "'";
  else if (s[0] == '"')
    return "\"";
  return "";
}

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

Node::Node() : _parent(nullptr)
{
  Node * root = hit::parse("blankline", "# blankline");
  const auto & first_child = *root->_children.front();
  _dhi = first_child._dhi;
  _hnv = first_child._hnv;
  _hnv.set_type(wasp::BLANK_LINE);
  delete root;
}

Node::Node(std::shared_ptr<wasp::DefaultHITInterpreter> dhi, wasp::HITNodeView hnv)
  : _dhi(dhi), _hnv(hnv), _parent(nullptr)
{
}

Node::Node(const std::string & field, const std::string & val) : _parent(nullptr)
{
  std::string input = field + " = " + val;
  Node * root = hit::parse("field", input);
  const auto & first_child = *root->_children.front();
  _dhi = first_child._dhi;
  _hnv = first_child._hnv;
  delete root;
}

Node::Node(const std::string & section) : _parent(nullptr)
{
  std::string input = "[" + (section.empty() ? "-" : section) + "][]";
  Node * root = hit::parse("section", input);
  const auto & first_child = *root->_children.front();
  _dhi = first_child._dhi;
  _hnv = first_child._hnv;
  for (auto child : first_child._children)
    addChild(child->clone());
  delete root;
}

Node::Node(const std::string & comment, bool is_inline) : _parent(nullptr)
{
  (void)is_inline; // suppress unused variable warning
  Node * root = hit::parse("comment", comment);
  const auto & first_child = *root->_children.front();
  _dhi = first_child._dhi;
  _hnv = first_child._hnv;
  delete root;
}

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

void
Node::remove()
{
  delete this;
}

wasp::HITNodeView
Node::getNodeView()
{
  return _hnv;
}

const std::string &
Node::filename()
{
  return _dhi->stream_name();
}

int
Node::line()
{
  return _hnv.line();
}

int
Node::column()
{
  return _hnv.column();
}

#define valthrow() throw Error("non-field node '" + fullpath() + "' has no value to retrieve")

bool
Node::boolVal()
{
  valthrow();
}
int64_t
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
std::vector<bool>
Node::vecBoolVal()
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
  if (_hnv.type() == wasp::BLANK_LINE)
    return NodeType::Blank;
  else if (_hnv.type() == wasp::COMMENT)
    return NodeType::Comment;
  if (_hnv.type() == wasp::OBJECT || _hnv.type() == wasp::DOCUMENT_ROOT)
    return NodeType::Section;
  else if (_hnv.type() == wasp::KEYED_VALUE || _hnv.type() == wasp::ARRAY)
    return NodeType::Field;
  return NodeType::Other;
}

void
Node::addChild(Node * child)
{
  child->_parent = this;
  _children.push_back(child);
}

void
Node::insertChild(std::size_t index, Node * child)
{
  child->_parent = this;
  _children.insert(_children.begin() + index, child);
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
Node::render(int indent, const std::string & indent_text, int maxlen)
{
  if (root() != this)
    indent = -1;

  std::string s;
  for (auto child : _children)
    s += child->render(indent + 1, indent_text, maxlen) + "\n";
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
  if (!_override_path.empty())
    return _override_path;
  std::string node_name = std::string(_hnv.name());
  return _hnv.type() == wasp::DOCUMENT_ROOT ? "" : node_name;
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
Node::walk(Walker * w, NodeType t, TraversalOrder o)
{
  if (!_dhi->size())
    return;

  // traverse children first
  if (o == TraversalOrder::AfterChildren)
    for (auto child : _children)
      child->walk(w, t, o);

  // execute walker
  if (type() == t || t == NodeType::All)
    w->walk(fullpath(), pathNorm(path()), this);

  // traverse children last
  if (o == TraversalOrder::BeforeChildren)
    for (auto child : _children)
      child->walk(w, t, o);
}

Node *
Node::find(const std::string & path)
{
  std::stringstream search(path);
  std::vector<Node *> parent_nodes{this};

  for (std::string name; std::getline(search, name, '/');)
  {
    if (name.empty())
      continue;

    std::vector<Node *> child_nodes;
    for (auto parent_node : parent_nodes)
      for (auto child : parent_node->children())
        if (child->path() == name)
          child_nodes.push_back(child);

    parent_nodes.clear();
    parent_nodes = child_nodes;

    if (parent_nodes.empty())
      break;
  }

  if (!parent_nodes.empty())
    return parent_nodes.front();
  return nullptr;
}

Comment::Comment(const std::string & text, bool is_inline = false)
  : Node(text, is_inline), _isinline(is_inline)
{
}

Comment::Comment(std::shared_ptr<wasp::DefaultHITInterpreter> dhi, wasp::HITNodeView hnv)
  : Node(dhi, hnv), _isinline(false)
{
}

std::string
Comment::render(int indent, const std::string & indent_text, int /*maxlen*/)
{
  const auto & comment_text = _hnv.data();

  if (_isinline)
    return " " + comment_text;
  return "\n" + strRepeat(indent_text, indent) + comment_text;
}

Node *
Comment::clone(bool
                   absolute_path
)
{
  auto n = new Comment(_dhi, _hnv);
  n->setInline(_isinline);
  if (absolute_path)
    n->setOverridePath(fullpath());
  return n;
}

void
Comment::setText(const std::string & text)
{
  if (text != _hnv.data() && _hnv.is_leaf())
    _hnv.set_data(text.c_str());
}

void
Comment::setInline(bool is_inline)
{
  _isinline = is_inline;
}

Section::Section(const std::string & path) : Node(path) {}

Section::Section(std::shared_ptr<wasp::DefaultHITInterpreter> dhi, wasp::HITNodeView hnv)
  : Node(dhi, hnv)
{
}

void
Section::clearLegacyMarkers()
{
  if (_hnv.child_count() > 3 && _hnv.child_at(2).type() == wasp::DECL && _hnv.child_at(1).is_leaf())
    _hnv.child_at(1).set_data("");

  auto term_node = _hnv.first_child_by_name("term");
  if (!term_node.is_null() && term_node.is_leaf())
    term_node.set_data("[]");
}

std::string
Section::path()
{
  return Node::path();
}

std::string
Section::render(int indent, const std::string & indent_text, int maxlen)
{
  std::string s;

  if (!path().empty() && path() != "-")
  {
    std::string opening_marks;
    if (_hnv.child_count() > 3 && _hnv.child_at(2).type() == wasp::DECL &&
        _hnv.child_at(1).is_leaf())
      opening_marks = _hnv.child_at(1).data();
    std::string decl_data = "[" + opening_marks + path() + "]";
    s = "\n" + strRepeat(indent_text, indent) + decl_data;
  }

  for (auto child : children())
  {
    if (!path().empty() && path() != "-")
      s += child->render(indent + 1, indent_text, maxlen);
    else
      s += child->render(indent, indent_text, maxlen);
  }

  if (!path().empty() && path() != "-")
  {
    wasp::HITNodeView term_node = _hnv.first_child_by_name("term");
    std::string term_data = !term_node.is_null() ? term_node.data() : "[]";
    s += "\n" + strRepeat(indent_text, indent) + term_data;
  }

  if (indent == 0 &&
      ((root() == this && s[0] == '\n') || (parent() && parent()->children()[0] == this)))
    s = s.substr(1);

  return s;
}

Node *
Section::clone(bool absolute_path)
{
  auto n = new Section(_dhi, _hnv);
  if (absolute_path)
    n->setOverridePath(fullpath());

  for (auto child : children())
    n->addChild(child->clone());
  return n;
}

Field::Field(const std::string & field, Kind k, const std::string & val)
  : Node(field, val), _kind(k)
{
}

Field::Field(std::shared_ptr<wasp::DefaultHITInterpreter> dhi, wasp::HITNodeView hnv)
  : Node(dhi, hnv), _kind(Kind::None)
{
}

std::string
Field::path()
{
  return Node::path();
}

std::string
Field::render(int indent, const std::string & indent_text, int maxlen)
{
  auto render_field = path();
  auto render_val = val();
  auto render_kind = kind();

  std::string s = "\n" + strRepeat(indent_text, indent) + render_field + " = ";

  size_t prefix_len = s.size() - 1;
  auto quote = quoteChar(render_val);
  int max = maxlen - prefix_len - 1;

  // special rendering logic for double quoted strings that go over maxlen:
  if (render_kind == Kind::String && quote == "\"" && max > 0)
  {
    if (render_val.find('\n') == std::string::npos)
    {
      // strip outer quotes - will will add back our own for each line
      std::string unquoted = render_val.substr(1, render_val.size() - 2);

      // iterate over the string in chunks of size "max"
      size_t pos = 0;
      while (pos + max < unquoted.size())
      {
        // to avoid splitting words, walk backwards from the "max" sized chunk boundary to find a
        // space character
        const std::string space = " \t";
        size_t boundary = pos + max;
        while (boundary > pos && space.find(unquoted[boundary]) == std::string::npos)
          boundary--;

        // if we didn't find a space, just fall back to the original max sized chunk boundary and
        // split the word anyway
        if (boundary == pos)
          boundary = pos + max;

        // shift the boundary to after the space character (instead of before it) unless that
        // would make the index beyond the string length.
        boundary = std::min(boundary + 1, unquoted.size());

        // add the leading indentation and newline - skip it for the first chunk of a string
        // because it should go on the same line as the "=",
        if (pos > 0)
          s += "\n" + strRepeat(" ", prefix_len);

        // add the quoted chunk to our string text
        s += quote + unquoted.substr(pos, boundary - pos) + quote;
        pos = boundary;
      }

      // add any remaining partial chunk of the string value
      if (pos < unquoted.size())
      {
        // again only add leading newline and indentation for greater chunks after the first.
        if (pos > 0)
          s += "\n" + strRepeat(" ", prefix_len);
        s += quote + unquoted.substr(pos, std::string::npos) + quote;
      }
    }
    else
    {
      const int delta_indent = _hnv.child_count() > 2 ? prefix_len - _hnv.child_at(2).column() : 0;

      // first line is always added as is
      std::size_t start = 0;
      std::size_t end = render_val.find('\n', start);
      s += render_val.substr(start, end - start + 1);

      // remaining lines
      do
      {
        start = end + 1;
        end = render_val.find('\n', start);
        if (end == std::string::npos)
          end = render_val.length() - 1;

        // remove leading whitespace
        const auto old_start = start;
        while (render_val[start] == ' ' || render_val[start] == '\t')
          ++start;

        // correct indentation
        if (delta_indent < 0)
        {
          if (old_start - delta_indent - 1 < start)
            start = old_start - delta_indent - 1;
        }
        else
        {
          start = old_start;
          s += std::string(delta_indent + 1, ' ');
        }

        s += render_val.substr(start, end - start + 1);
      } while (end < render_val.length() - 1);
    }
  }
  else if (render_val.size() == 0)
    s += "''";
  else if (quote == "" && render_val.find_first_of("\n\r \t") != std::string::npos)
    s += "'" + render_val + "'";
  else
    s += render_val;

  for (auto child : children())
    s += child->render(indent + 1, indent_text, maxlen);

  return s;
}

Node *
Field::clone(bool absolute_path)
{
  auto n = new Field(_dhi, _hnv);
  if (absolute_path)
    n->setOverridePath(fullpath());
  return n;
}

Field::Kind
Field::kind()
{
  if (_kind != Kind::None)
    return _kind;

  Kind value_kind = Kind::String;

  try
  {
    boolVal();
    value_kind = Kind::Bool;
  }
  catch (...)
  {
  }

  try
  {
    floatVal();
    value_kind = Kind::Float;
  }
  catch (...)
  {
  }

  try
  {
    intVal();
    value_kind = Kind::Int;
  }
  catch (...)
  {
  }

  return value_kind;
}

void
Field::setVal(const std::string & value, Kind kind)
{
  if (kind != Kind::None)
    _kind = kind;

  if (value == val())
    return;

  wasp::HITNodeView node = _hnv;

  if (node.type() == wasp::KEYED_VALUE || node.type() == wasp::ARRAY)
  {
    wasp::HITNodeView data_node;

    for (std::size_t i = 0, count = node.child_count(); i < count; i++)
    {
      auto ch = node.child_at(i);
      if (i > 1 || (ch.type() != wasp::DECL && ch.type() != wasp::ASSIGN))
        if (ch.is_leaf())
        {
          ch.set_data("");
          if (data_node.is_null())
            data_node = ch;
        }
    }

    node = data_node;
  }

  if (node.is_leaf())
    node.set_data(value.c_str());
}

std::string
Field::val()
{
  std::string value = _hnv.data();

  size_t equal_index = value.find("=");
  if (equal_index != std::string::npos)
    value = value.substr(equal_index + 1);

  value = hit::trim(value);
  std::string quote = quoteChar(value);
  if (quote.empty())
    return value;

  std::string chain;
  size_t start_pos = 0;
  size_t found_pos = value.find_first_of("\\" + quote, start_pos);
  bool inside_quotes = false;

  while (found_pos != std::string::npos)
  {
    if (inside_quotes)
      chain += value.substr(start_pos, found_pos - start_pos);

    start_pos = found_pos;

    if (value[start_pos] == '\\')
    {
      chain += "\\";
      start_pos++;
      chain += value[start_pos];
      start_pos++;
    }
    else // value[start_pos] == (non-escaped) 'quote'
    {
      start_pos++;
      inside_quotes = !inside_quotes;
      if (!inside_quotes)
        start_pos = value.find_first_not_of(" \r\n\t", start_pos);
    }

    found_pos = value.find_first_of("\\" + quote, start_pos);
  }

  return quote + chain + quote;
}

std::vector<bool>
Field::vecBoolVal()
{
  std::vector<bool> vec;
  for (const auto & s : vecStrVal())
  {
    bool converted_val = false;
    if (!toBool(s, &converted_val))
      throw Error("cannot convert field '" + fullpath() + "' value '" + s + "' to bool");
    vec.push_back(converted_val);
  }
  return vec;
}

std::vector<int>
Field::vecIntVal()
{
  std::vector<int> vec;
  for (const auto & s : vecStrVal())
  {
    std::istringstream iss(s);
    int conversion;
    if ((iss >> conversion).fail() || !iss.eof())
      throw Error("cannot convert field '" + fullpath() + "' value '" + s + "' to integer");
    vec.push_back(conversion);
  }
  return vec;
}

std::vector<double>
Field::vecFloatVal()
{
  std::vector<double> vec;
  for (const auto & s : vecStrVal())
  {
    std::istringstream iss(s);
    double conversion;
    if ((iss >> conversion).fail() || !iss.eof())
      throw Error("cannot convert field '" + fullpath() + "' value '" + s + "' to float");
    vec.push_back(conversion);
  }
  return vec;
}

std::vector<std::string>
Field::vecStrVal()
{
  std::string unquoted = val();
  if (!quoteChar(unquoted).empty())
    unquoted = unquoted.substr(1, unquoted.size() - 2);
  return split(unquoted);
}

bool
Field::boolVal()
{
  bool bool_value = false;
  try
  {
    int int_value = intVal();
    return int_value;
  }
  catch (...)
  {
    std::string str_value = val();
    if (!toBool(str_value, &bool_value))
      throw Error("field node '" + fullpath() + "' does not hold a bool-typed value (val='" +
                  str_value + "')");
  }
  return bool_value;
}

int64_t
Field::intVal()
{
  std::string str_value = val();
  std::istringstream iss(str_value);
  int64_t converted_val;
  if ((iss >> converted_val).fail() || !iss.eof())
    throw Error("cannot convert field '" + fullpath() + "' value '" + str_value + "' to integer");
  return converted_val;
}

double
Field::floatVal()
{
  std::string str_value = val();
  std::istringstream iss(str_value);
  double converted_val;
  if ((iss >> converted_val).fail() || !iss.eof())
    throw Error("cannot convert field '" + fullpath() + "' value '" + str_value + "' to float");
  return converted_val;
}

std::string
Field::strVal()
{
  const auto value = val();
  std::string s = value;

  const auto quote = quoteChar(value);
  if (quote != "")
  {
    s = value.substr(1, value.size() - 2);

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

// parse tokenizes the given hit input from fname using a Lexer with the lexHit as the
// starting LexFunc.  Parsing is implemented as recursive-descent with the functions in this file
// named "parse[Bla]".
Node *
parse(const std::string & fname, const std::string & input)
{
  std::stringstream input_errors;
  std::shared_ptr<wasp::DefaultHITInterpreter> interpreter =
      std::make_shared<wasp::DefaultHITInterpreter>(input_errors);

  std::stringstream input_stream(input);
  if (!interpreter->parseStream(input_stream, fname))
    throw ParseError(input_errors.str());

  std::unique_ptr<Node> root(new Section(interpreter, interpreter->root()));
  std::size_t starting_line = 0;
  buildHITTree(interpreter, interpreter->root(), root.get(), starting_line);

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
      // node exists - overwrite its value and kind
      auto dst = static_cast<Field *>(result);
      auto src = static_cast<Field *>(n);
      dst->setVal(src->val(), src->kind());
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
  void walk(const std::string & /*fullpath*/, const std::string & /*nodepath*/, Node * n) override
  {
    auto result = _orig->find(n->fullpath());
    if (!result && n->parent())
    {
      auto anchor = _orig->find(n->parent()->fullpath());
      if (anchor)
        anchor->addChild(n->clone());
    }
  }

  NodeType nodeType() override { return NodeType::Section; }

private:
  std::set<std::string> _done;
  Node * _orig;
};

void
merge(Node * from, Node * into)
{
  MergeFieldWalker fw(into);
  MergeSectionWalker sw(into);
  from->walk(&fw);
  from->walk(&sw);
}

Node *
explode(Node * n)
{
  /// all of the explode logic that expands shorthand path notation
  /// into sections is handled by a combination of the wasp interpreter and
  /// the buildHITTree so this method is a no-op and is temporarily here
  /// for interface purposes
  return n->root();
}

/// when a node tree is walked with this, all legacy markers in section
/// headers './' and legacy markers in section closers '../'are removed
class LegacyMarkerRemover : public Walker
{
public:
  void walk(const std::string & /*fullpath*/, const std::string & /*nodepath*/, Node * n) override
  {
    static_cast<Section *>(n)->clearLegacyMarkers();
  }
};

// matches returns true if s matches the given regex pattern.
bool
matches(const std::string & s, const std::string & regex, bool full = true)
{
  try
  {
    if (full)
      return std::regex_match(s, std::regex(regex));
    return std::regex_search(s, std::regex(regex));
  }
  catch (...)
  {
    return false;
  }
}

Formatter::Formatter() : canonical_section_markers(true), line_length(100), indent_string("  ") {}

void
Formatter::walkPatternConfig(const std::string & prefix, Node * n)
{
  std::vector<std::string> order;
  for (auto child : n->children())
  {
    order.push_back(child->path());
    if (child->type() == NodeType::Section)
    {
      auto subpath = prefix + "/" + child->path();
      if (prefix == "")
        subpath = child->path();
      walkPatternConfig(subpath, child);
    }
  }

  addPattern(prefix, order);
}

Formatter::Formatter(const std::string & fname, const std::string & hit_config)
  : canonical_section_markers(true), line_length(100), indent_string("  ")
{
  std::unique_ptr<hit::Node> root(hit::parse(fname, hit_config));
  if (root->find("format/indent_string"))
    indent_string = root->param<std::string>("format/indent_string");
  if (root->find("format/line_length"))
    line_length = root->param<int>("format/line_length");
  if (root->find("format/canonical_section_markers"))
    canonical_section_markers = root->param<bool>("format/canonical_section_markers");
  if (root->find("format/sorting"))
    walkPatternConfig("", root->find("format/sorting"));
}

std::string
Formatter::format(const std::string & fname, const std::string & input)
{
  std::unique_ptr<hit::Node> root(hit::parse(fname, input));
  format(root.get());
  return root->render(0, indent_string, line_length);
}

void
Formatter::format(hit::Node * root)
{
  LegacyMarkerRemover walker;
  if (canonical_section_markers)
    root->walk(&walker, hit::NodeType::Section);

  root->walk(this, hit::NodeType::All);
}

void
Formatter::addPattern(const std::string & section, const std::vector<std::string> & order)
{
  _patterns.push_back({section, order});
}

void
Formatter::walk(const std::string & fullpath, const std::string & /*nodepath*/, Node * n)
{
  for (auto & pattern : _patterns)
  {
    if (!matches(fullpath, pattern.regex, true))
      continue;

    std::vector<std::string> frontorder;
    std::vector<std::string> backorder;
    bool onfront = true;
    for (auto & field : pattern.order)
    {
      if (field == "**")
      {
        onfront = false;
        continue;
      }
      else if (onfront)
        frontorder.push_back(field);
      else
        backorder.push_back(field);
    }

    auto nodes = n->children();
    std::vector<Node *> fronthalf;
    std::vector<Node *> unused;
    sortGroup(nodes, frontorder, fronthalf, unused);

    std::vector<Node *> backhalf;
    nodes = unused;
    unused.clear();
    sortGroup(nodes, backorder, backhalf, unused);

    std::vector<Node *> children;
    children.insert(children.end(), fronthalf.begin(), fronthalf.end());
    children.insert(children.end(), unused.begin(), unused.end());
    children.insert(children.end(), backhalf.rbegin(), backhalf.rend());

    for (unsigned int i = 0; i < children.size(); i++)
      children[i] = children[i]->clone();

    for (auto child : n->children())
      delete child;

    for (auto child : children)
      n->addChild(child);
  }
}

void
Formatter::sortGroup(const std::vector<Node *> & nodes,
                     const std::vector<std::string> & order,
                     std::vector<Node *> & sorted,
                     std::vector<Node *> & unused)
{
  std::vector<bool> skips(nodes.size(), false);

  for (auto next : order)
  {
    for (unsigned int i = 0; i < nodes.size(); i++)
    {
      if (skips[i])
        continue;

      auto comment = nodes[i];
      Node * field = nullptr;
      if (i + 1 < nodes.size())
        field = nodes[i + 1];

      if ((comment->type() == NodeType::Comment || comment->type() == NodeType::Blank) && field &&
          (field->type() == NodeType::Field || field->type() == NodeType::Section))
        i++;
      else if (comment->type() == NodeType::Field || comment->type() == NodeType::Section)
      {
        field = comment;
        comment = nullptr;
      }
      else
        continue;

      if (matches(next, field->path(), false))
      {
        if (comment != nullptr)
        {
          sorted.push_back(comment);
          skips[i + 1] = true;
        }
        skips[i] = true;
        sorted.push_back(field);
      }
    }
  }

  for (unsigned int i = 0; i < skips.size(); i++)
    if (skips[i] == false)
      unused.push_back(nodes[i]);
}

void
buildHITTree(std::shared_ptr<wasp::DefaultHITInterpreter> interpreter,
             wasp::HITNodeView hnv_parent,
             Node * hit_parent,
             std::size_t & previous_line)
{
  if (hnv_parent.is_null())
    return;

  for (std::size_t i = 0, count = hnv_parent.child_count(); i < count; i++)
  {
    // wasp nodeview child will be stored in node if one is created and added
    auto hnv_child = hnv_parent.child_at(i);

    // create and add comment node as terminal leaf but do not recurse deeper
    if (hnv_child.type() == wasp::COMMENT)
    {
      // add blank line if needed between previous node line and this node line
      if (previous_line != 0 && hnv_child.line() > previous_line + 1)
        hit_parent->addChild(new Blank());

      auto hit_child = new Comment(interpreter, hnv_child);
      if (hnv_child.line() == previous_line && hit_parent->children().size() > 0)
      {
        hit_child->setInline(true);
        hit_parent->children().back()->addChild(hit_child);
      }
      else
      {
        hit_child->setInline(false);
        hit_parent->addChild(hit_child);
      }
      previous_line = hnv_child.last_line();
    }
    // create and add field node as a combined leaf but do not recurse deeper
    else if (hnv_child.type() == wasp::KEYED_VALUE || hnv_child.type() == wasp::ARRAY)
    {
      // add blank line if needed between previous node line and this node line
      if (previous_line != 0 && hnv_child.line() > previous_line + 1)
        hit_parent->addChild(new Blank());

      auto hit_child = new Field(interpreter, hnv_child);
      hit_parent->addChild(hit_child);
      previous_line = hnv_child.last_line();
    }
    // section handling is dependant on if specification is explode or normal
    else if (hnv_child.type() == wasp::OBJECT || hnv_child.type() == wasp::DOCUMENT_ROOT)
    {
      bool explode_section_found = false;

      // explode section found so create and add nothing just recurse with it
      wasp::HITNodeView decl_node = hnv_child.first_child_by_name("decl");

      if (decl_node.is_null() || decl_node.data().find("/") != std::string::npos)
      {
        if (Node * hit_child = hit_parent->find(hnv_child.name()))
        {
          buildHITTree(interpreter, hnv_child, hit_child, previous_line);
          explode_section_found = true;
        }
      }

      // normal or absent section so create and add node then recurse with it
      if (!explode_section_found)
      {
        // add blank line if needed between previous node line and this node line
        if (previous_line != 0 && hnv_child.line() > previous_line + 1)
          hit_parent->addChild(new Blank());

        auto hit_child = new Section(interpreter, hnv_child);
        hit_parent->addChild(hit_child);
        previous_line = hnv_child.line();
        buildHITTree(interpreter, hnv_child, hit_child, previous_line);
        previous_line = hnv_child.last_line();
      }
    }
  }
}

} // namespace hit
