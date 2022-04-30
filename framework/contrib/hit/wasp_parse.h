#ifndef HIT_WASP_PARSE
#define HIT_WASP_PARSE

#include <algorithm>
#include <iterator>
#include <memory>
#include <regex>
#include <exception>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "wasphit/HITInterpreter.h"
#include "wasphit/HITNodeView.h"

namespace wasp_hit
{

// forward declarations

class Node;

Node * parse(const std::string & fname, const std::string & input);

std::vector<std::string> split(const std::string & input);

std::string trim(const std::string & str);

bool toBool(const std::string & val, bool * dst);

std::string quoteChar(const std::string & s);

std::string strRepeat(const std::string & s, int n);

std::string pathNorm(const std::string & path);

/// Error is the superclass for all hit parser related errors. this includes errors for
/// requesting values of the wrong type from a parsed hit tree.
class Error : public std::exception
{
public:
  Error(const std::string & msg) : msg(msg) {}

  virtual const char * what() const throw() { return msg.c_str(); }

  std::string msg;
};

/// ParseError represents a parsing error (i.e. bad syntax, invalid characters, etc.).
class ParseError : public Error
{
public:
  ParseError(const std::string & msg) : Error(msg) {}
};

/// NodeType represents every element type in a parsed hit tree.
enum class NodeType
{
  All,     /// used for tree-traversal/manipulation to indicate all functions.
  Root,    /// represents the root, most un-nested node of a parsed hit tree.
  Section, /// represents hit sections (i.e. "[pathname]...[../]").
  Comment, /// represents comments that are not directly part of the actual hit document.
  Field,   /// represents field-value pairs (i.e. paramname=val).
  Blank,   /// represents a blank line
  Other,   /// represents any other type of node
};

/// traversal order for walkers. determines if the walker on a node is executed before or
/// after the child nodes were traversed.
enum class TraversalOrder
{
  BeforeChildren, /// walker is executed then child nodes are traversed.
  AfterChildren   /// child nodes are traversed then walker is executed.
};

/// Walker is an interface that can be implemented to perform operations that traverse a
/// parsed hit node tree. implementing classes are passed to the Node::walk function.
class Walker
{
public:
  virtual ~Walker() {}

  /// walk is called when the walker is passed into a Node::walk function for each relevant node in
  /// the hit (sub)tree. fullpath is the fully-qualified (absolute) path to the hit node
  /// where each section header is a path-element. nodepath is the path for the node of interest -
  /// section name for Section nodes and field/parameter name for Field nodes. n is the actual
  /// node.
  virtual void walk(const std::string & fullpath, const std::string & nodepath, Node * n) = 0;

  /// return the default node type this walker should be applied to
  virtual NodeType nodeType() { return NodeType::Field; }

  /// return the default traversal order
  virtual TraversalOrder traversalOrder() { return TraversalOrder::BeforeChildren; }
};

/// Node represents an object in a parsed hit tree. each node manages the memory for its child
/// nodes. it is safe to delete any node in the tree; doing so will also delete that node's
/// children recursively. it is not safe to place a single node in multiple trees. instead, use
/// the node's clone function (which operates recursively) to create a new (sub)tree for placement
/// in alternate trees.
class Node
{
public:
  /// constructs a Node from wasp interpreter shared pointer and nodeview
  Node(std::shared_ptr<wasp::DefaultHITInterpreter> dhi, wasp::HITNodeView hnv)
    : _dhi(dhi), _hnv(hnv), _parent(nullptr)
  {
  }

  /// constructs a Node from field string, kind enum, and value string
  Node(const std::string & field, const std::string & val) : _parent(nullptr)
  {
    std::string input = field + " = " + val;

    Node * root = wasp_hit::parse("field", input);

    root = root->_children.front();

    _dhi = root->_dhi;

    _hnv = root->_hnv;

    delete root;
  }

  /// constructs a Node from section name - named 'empty' if name is empty
  Node(const std::string & section) : _parent(nullptr)
  {
    std::string input = "[" + (section.empty() ? "-" : section) + "][]";

    Node * root = wasp_hit::parse("section", input);

    root = root->_children.front();

    _dhi = root->_dhi;

    _hnv = root->_hnv;

    for (auto child : root->_children)
    {
      addChild(child->clone());
    }

    delete root;
  }

  /// constructs a Node from comment text - is_inline is currently unused
  Node(const std::string & comment, bool is_inline) : _parent(nullptr)
  {
    (void)is_inline; // suppress unused variable warning

    Node * root = wasp_hit::parse("comment", comment);

    root = root->_children.front();

    _dhi = root->_dhi;

    _hnv = root->_hnv;

    delete root;
  }

  /// destructs and frees memory for this node and all recursive children
  virtual ~Node()
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

  /// destructs and frees memory for this node and all recursive children
  void remove() { delete this; }

  /// path returns this node's local/direct contribution its full hit path. for section nodes, this
  /// is the section name, for field nodes, this is the field/parameter name, for other nodes this
  /// is empty. if set_override_path has been called for this Node, then that set path is returned.
  virtual std::string path()
  {
    if (!_override_path.empty())
    {
      return _override_path;
    }

    std::string node_name = std::string(_hnv.name());

    return node_name == "/" ? "" : node_name;
  }

  /// fullpath returns the full hit path to this node (including all parent sections
  /// recursively) starting from the tree's root node.
  std::string fullpath()
  {
    if (_parent == nullptr)
      return "";

    auto ppath = _parent->fullpath();
    if (ppath.empty())
      return path();
    return _parent->fullpath() + "/" + path();
  }

  /// type returns the type of the node (e.g. one of Field, Section, Comment, etc.)
  NodeType type()
  {
    NodeType node_type = NodeType::Other;

    if (_hnv.type() == wasp::COMMENT)
    {
      node_type = NodeType::Comment;
    }
    else if (_hnv.type() == wasp::OBJECT || _hnv.type() == wasp::DOCUMENT_ROOT)
    {
      node_type = NodeType::Section;
    }
    else if (_hnv.type() == wasp::KEYED_VALUE || _hnv.type() == wasp::ARRAY)
    {
      node_type = NodeType::Field;
    }

    return node_type;
  }

  /// name returns the file name of the original parsed input (file) that contained the start of
  /// the content that this node was built from.
  const std::string & filename() { return _dhi->stream_name(); }

  /// line returns the line number of the original parsed input (file) that contained the start of
  /// the content that this node was built from.
  int line() { return _hnv.line(); }

  /// render builds an hit syntax/text that is equivalent to the hit tree starting at this
  /// node (and downward) - i.e. parsing this function's returned string would yield a node tree
  /// identical to this nodes tree downward. indent is the indent level using indent_text as the
  /// indent string (repeated once for each level). maxlen is the maximum line length before
  /// breaking string values.
  virtual std::string render(int indent = 0,
                             const std::string & indent_text = "  ",
                             int maxlen = 0,
                             int parent_newline_count = 0)
  {
    (void)parent_newline_count; // suppress unused variable warning

    if (root() != this)
    {
      indent = -1;
    }

    std::string s;

    for (auto child : _children)
    {
      s += child->render(indent + 1, indent_text, maxlen) + "\n";
    }

    return s;
  }

  /// clone returns a complete (deep) copy of this node. the caller will be responsible for
  /// managing the memory/deallocation of the returned clone node.
  virtual Node * clone(bool absolute_path = false) = 0;

  /// set_override_path supplies this Node with an alternative string to use
  /// rather than its regular name in all calls to the path requester method
  void set_override_path(const std::string & override_path) { _override_path = override_path; }

  /// methods returning underlying value data as specific types should only
  /// be called for Field Nodes and are defined in the derived Field class.
#define valthrow() throw Error("non-field node '" + fullpath() + "' has no value to retrieve")
  virtual bool boolVal() { valthrow(); }
  virtual int64_t intVal() { valthrow(); }
  virtual double floatVal() { valthrow(); }
  virtual std::string strVal() { valthrow(); }
  virtual std::vector<bool> vecBoolVal() { valthrow(); }
  virtual std::vector<int> vecIntVal() { valthrow(); }
  virtual std::vector<double> vecFloatVal() { valthrow(); }
  virtual std::vector<std::string> vecStrVal() { valthrow(); }
#undef valthrow

  /// addChild adds a node to the ordered set of this node's children. this node assumes/takes
  /// ownership of the memory of the passed child.
  void addChild(Node * child)
  {
    child->_parent = this;
    _children.push_back(child);
  }

  /// insertChild inserts a node prior to the supplied index to the ordered set of this node's
  /// children. this node assumes/takes ownership of the memory of the passed child.
  void insertChild(std::size_t index, Node * child)
  {
    child->_parent = this;
    _children.insert(_children.begin() + index, child);
  }

  /// children returns a list of this node's children of the given type t.
  std::vector<Node *> children(NodeType t = NodeType::All)
  {
    if (t == NodeType::All)
      return _children;
    std::vector<Node *> nodes;
    for (auto child : _children)
      if (child->type() == t)
        nodes.push_back(child);
    return nodes;
  }

  /// parent returns a pointer to this node's parent node or nullptr if this node has no parent.
  Node * parent() { return _parent; }

  /// root returns the root node for the hit tree this node resides in.
  Node * root()
  {
    if (_parent == nullptr)
      return this;
    return _parent->root();
  }

  /// walk does a depth-first traversal of the hit tree starting at this node (it
  /// doesn't visit any nodes that require traversing this node's parent) calling the passed
  /// walker's walk function for each node visited. w->walk is not called for nodes that are not
  /// of type t although nodes not of type t are still traversed.
  void walk(Walker * w) { walk(w, w->nodeType(), w->traversalOrder()); }
  void walk(Walker * w, NodeType t) { walk(w, t, w->traversalOrder()); }
  void walk(Walker * w, NodeType t, TraversalOrder o)
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

  /// find follows the tree along the given path starting at this node (downward not checking any
  /// nodes that require traversing this node's parent) and returns the first node it finds at the
  /// given relative path if any or nullptr otherwise.
  Node * find(const std::string & path)
  {
    std::stringstream search(path);

    std::vector<Node *> parent_nodes;

    parent_nodes.push_back(this);

    for (std::string name; std::getline(search, name, '/');)
    {
      if (name.empty())
        continue;

      std::vector<Node *> child_nodes;

      for (auto parent_node : parent_nodes)
      {
        auto children = parent_node->children();

        for (auto child : children)
        {
          if (child->path() == name)
          {
            child_nodes.push_back(child);
          }
        }
      }

      parent_nodes.clear();

      parent_nodes = child_nodes;

      if (parent_nodes.empty())
        break;
    }

    if (!parent_nodes.empty())
    {
      return parent_nodes.front();
    }
    else
    {
      return nullptr;
    }
  }

  /// param searches for the node at the given path (empty path indicates *this* node) and returns
  /// the value stored at that node in the form of the given type T. the node at the given path
  /// must hold a value (i.e. be a Field node) otherwise an exception is thrown. if the node holds
  /// a value that cannot be represented as type T, an exception is also thrown. all (field) nodes
  /// can return their value as a std::string type.
  template <typename T>
  T param(const std::string & path = "")
  {
    auto n = this;
    if (path != "")
      n = find(path);
    if (n == nullptr)
      throw Error("no parameter named '" + path + "'");
    return paramInner<T>(n);
  }

  /// paramOptional is identical to param except if no node is found at the given path, default_val
  /// is returned instead of the value returned by the param function.
  template <typename T>
  T paramOptional(const std::string & path, T default_val)
  {
    if (find(path) == nullptr)
      return default_val;
    return param<T>(path);
  }

protected:
  std::shared_ptr<wasp::DefaultHITInterpreter> _dhi;

  wasp::HITNodeView _hnv;

private:
  Node * _parent;

  std::vector<Node *> _children;

  std::string _override_path;

  template <typename T>
  T paramInner(Node *)
  {
    throw Error("unsupported c++ type '" + std::string(typeid(T).name()) + "'");
  }
};

template <>
inline bool
Node::paramInner(Node * n)
{
  return n->boolVal();
}

template <>
inline int64_t
Node::paramInner(Node * n)
{
  return n->intVal();
}

template <>
inline int
Node::paramInner(Node * n)
{
  return n->intVal();
}

template <>
inline unsigned int
Node::paramInner(Node * n)
{
  if (n->intVal() < 0)
    throw Error("negative value read from file '" + n->filename() + "' on line " +
                std::to_string(n->line()));
  return n->intVal();
}

template <>
inline float
Node::paramInner(Node * n)
{
  return n->floatVal();
}

template <>
inline double
Node::paramInner(Node * n)
{
  return n->floatVal();
}

template <>
inline std::string
Node::paramInner(Node * n)
{
  return n->strVal();
}

template <>
inline std::vector<bool>
Node::paramInner(Node * n)
{
  return n->vecBoolVal();
}

template <>
inline std::vector<int>
Node::paramInner(Node * n)
{
  return n->vecIntVal();
}

template <>
inline std::vector<unsigned int>
Node::paramInner(Node * n)
{
  auto tmp = n->vecIntVal();
  std::vector<unsigned int> vec;
  for (auto val : tmp)
  {
    if (val < 0)
      throw Error("negative value read from file '" + n->filename() + "' on line " +
                  std::to_string(n->line()));
    vec.push_back(val);
  }
  return vec;
}

template <>
inline std::vector<double>
Node::paramInner(Node * n)
{
  return n->vecFloatVal();
}

template <>
inline std::vector<float>
Node::paramInner(Node * n)
{
  auto tmp = n->vecFloatVal();
  std::vector<float> vec;
  for (auto val : tmp)
    vec.push_back(val);
  return vec;
}

template <>
inline std::vector<std::string>
Node::paramInner(Node * n)
{
  return n->vecStrVal();
}

/// Field represents the field-value pairs or parameters that provide the meat content of hit
/// documents. Fields have a name, a kind (e.g. Int, Bool, Float, String), and a value.
class Field : public Node
{
public:
  /// Kind represents all possible value types that can be stored in a field.
  enum class Kind : unsigned char
  {
    None,
    Bool,
    Int,
    Float,
    String,
  };

  /// constructs a Node from wasp interpreter shared pointer and nodeview
  Field(std::shared_ptr<wasp::DefaultHITInterpreter> dhi, wasp::HITNodeView hnv)
    : Node(dhi, hnv), _kind(Kind::None)
  {
  }

  /// constructs a Node from field string, kind enum, and value string
  Field(const std::string & field, Kind k, const std::string & val) : Node(field, val), _kind(k) {}

  /// path returns the hit Field name (i.e. content before the "=")
  virtual std::string path() override { return Node::path(); }

  /// render builds text with hit syntax that is equivalent to this field.
  virtual std::string render(int indent = 0,
                             const std::string & indent_text = "  ",
                             int maxlen = 0,
                             int parent_newline_count = 0) override
  {
    (void)parent_newline_count; // suppress unused variable warning

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
        const int delta_indent =
            _hnv.child_count() > 2 ? prefix_len - _hnv.child_at(2).column() : 0;

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

    return s;
  }

  /// clone returns a complete (deep) copy of this node. the caller will be responsible for
  /// managing the memory/deallocation of the returned clone node.
  virtual Node * clone(bool absolute_path = false) override
  {
    auto node = new Field(_dhi, _hnv);

    if (absolute_path)
    {
      node->set_override_path(fullpath());
    }

    return node;
  }

  /// kind returns the semantic type of the value stored in this field (e.g. Int, Bool, Float,
  /// String).
  Kind kind()
  {
    if (_kind != Kind::None)
    {
      return _kind;
    }

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

  /// setVal is a way to overwrite and set the field's underlying value content to anything. this
  /// does not affect the value returned by the kind function by default (e.g. setting a "Bool"
  /// field to "42" does not make the field's kind "Int" - it will just cause errors when you try
  /// to retrieve the value via anything other than strVal).
  void setVal(const std::string & value, Kind kind = Kind::None)
  {
    if (kind != Kind::None)
    {
      _kind = kind;
    }

    if (value == val())
    {
      return;
    }

    wasp::HITNodeView node = _hnv;

    if (node.type() == wasp::KEYED_VALUE || node.type() == wasp::ARRAY)
    {
      wasp::HITNodeView data_node;

      for (std::size_t i = 0, count = node.child_count(); i < count; i++)
      {
        auto ch = node.child_at(i);

        if (i > 1 || (ch.type() != wasp::DECL && ch.type() != wasp::ASSIGN))
        {
          if (ch.is_leaf())
          {
            ch.set_data("");

            if (data_node.is_null())
            {
              data_node = ch;
            }
          }
        }
      }

      node = data_node;
    }

    if (node.is_leaf())
    {
      node.set_data(value.c_str());
    }
  }

  /// val returns the raw text of the field's value as it was read from the
  /// hit input. this is the content after the "=" with whitespace trimmed.
  std::string val()
  {
    std::string value = _hnv.data();

    size_t equal_index = value.find("=");

    if (equal_index != std::string::npos)
    {
      value = value.substr(equal_index + 1);
    }

    value = wasp_hit::trim(value);

    std::string quote = quoteChar(value);

    if (quote.empty())
    {
      return value;
    }

    std::string chain;

    size_t start_pos = 0;

    size_t found_pos = value.find_first_of("\\" + quote, start_pos);

    bool inside_quotes = false;

    while (found_pos != std::string::npos)
    {
      if (inside_quotes)
      {
        chain += value.substr(start_pos, found_pos - start_pos);
      }

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
        {
          start_pos = value.find_first_not_of(" \r\n\t", start_pos);
        }
      }

      found_pos = value.find_first_of("\\" + quote, start_pos);
    }

    return quote + chain + quote;
  }

  /// the vec-prefixed value retrieval functions assume the node holds a string-typed value holding
  /// whitespace delimited entries of the element type indicated in the function name.

  virtual std::vector<bool> vecBoolVal() override
  {
    std::vector<bool> converted_values;

    auto str_values = vecStrVal();

    for (auto & str_value : str_values)
    {
      bool bool_value = false;

      if (!toBool(str_value, &bool_value))
      {
        throw Error("cannot convert field '" + fullpath() + "' value '" + str_value + "' to bool");
      }

      converted_values.push_back(bool_value);
    }

    return converted_values;
  }

  virtual std::vector<int> vecIntVal() override
  {
    std::vector<int> converted_values;

    auto str_values = vecStrVal();

    for (auto & str_value : str_values)
    {
      std::istringstream iss(str_value);

      int conversion;

      if ((iss >> conversion).fail() || !iss.eof())
      {
        throw Error("cannot convert field '" + fullpath() + "' value '" + str_value +
                    "' to integer");
      }

      converted_values.push_back(conversion);
    }

    return converted_values;
  }

  virtual std::vector<double> vecFloatVal() override
  {
    std::vector<double> converted_values;

    auto str_values = vecStrVal();

    for (auto & str_value : str_values)
    {
      std::istringstream iss(str_value);

      double conversion;

      if ((iss >> conversion).fail() || !iss.eof())
      {
        throw Error("cannot convert field '" + fullpath() + "' value '" + str_value + "' to float");
      }

      converted_values.push_back(conversion);
    }

    return converted_values;
  }

  virtual std::vector<std::string> vecStrVal() override
  {
    std::string array_data = val();

    if (!quoteChar(array_data).empty())
    {
      array_data = array_data.substr(1, array_data.size() - 2);
    }

    return split(array_data);
  }

  /// the following functions return the stored value of the node (if any exists) of the type
  /// indicated in the function name. if the node holds a value of a different type or doesn't hold
  /// a value at all, an exception will be thrown.

  virtual bool boolVal() override
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
      {
        throw Error("field node '" + fullpath() + "' does not hold a bool-typed value (val='" +
                    str_value + "')");
      }
    }
    return bool_value;
  }

  virtual int64_t intVal() override
  {
    std::string str_value = val();

    std::istringstream iss(str_value);

    int64_t conversion;

    if ((iss >> conversion).fail() || !iss.eof())
    {
      throw Error("cannot convert field '" + fullpath() + "' value '" + str_value + "' to integer");
    }

    return conversion;
  }

  virtual double floatVal() override
  {
    std::string str_value = val();

    std::istringstream iss(str_value);

    double conversion;

    if ((iss >> conversion).fail() || !iss.eof())
    {
      throw Error("cannot convert field '" + fullpath() + "' value '" + str_value + "' to float");
    }

    return conversion;
  }

  /// strVal is special in that it only throws an exception if the node doesn't hold a value at
  /// all. all nodes that have a value hold data that was originally represented as a string in
  /// the parsed input - so this returns that raw string and removes any existing outer quotes.
  virtual std::string strVal() override
  {
    std::string node_data = val();

    auto quote = quoteChar(node_data);

    if (quote != "")
    {
      node_data = node_data.substr(1, node_data.size() - 2);

      size_t pos = node_data.find("\\" + quote, 0);

      while (pos != std::string::npos)
      {
        node_data.replace(pos, 2, quote);
        pos += 1; // handles when replaced text is a substring of find text
        pos = node_data.find("\\" + quote, pos);
      }
    }
    return node_data;
  }

private:
  Kind _kind;
};

/// Section represents a hit section including the section header path and all entries inside
/// the section (e.g. fields/parameters, subsections, etc.).
class Section : public Node
{
public:
  /// constructs a Node from wasp interpreter shared pointer and nodeview
  Section(std::shared_ptr<wasp::DefaultHITInterpreter> dhi, wasp::HITNodeView hnv) : Node(dhi, hnv)
  {
  }

  /// constructs a Node from section name - named 'empty' if name is empty
  Section(const std::string & path) : Node(path) {}

  /// path returns the hit path located in the section's header i.e. the section's name.
  virtual std::string path() override { return Node::path(); }

  /// render builds text with hit syntax that is equivalent to this section.
  virtual std::string render(int indent = 0,
                             const std::string & indent_text = "  ",
                             int maxlen = 0,
                             int parent_newline_count = 0) override
  {
    std::string s;

    if (path() != "document" && path() != "-")
    {
      std::string opening_marks;

      if (_hnv.child_count() > 3 && _hnv.child_at(2).type() == wasp::DECL &&
          _hnv.child_at(1).is_leaf())
      {
        opening_marks = _hnv.child_at(1).data();
      }

      std::string decl_data = "[" + opening_marks + path() + "]";

      append_blank_lines(s, _hnv.line(), parent_newline_count);

      s = "\n" + strRepeat(indent_text, indent) + decl_data;
    }

    for (auto child : children())
    {
      int current_newline_count = append_blank_lines(s, child->line(), parent_newline_count);

      if (path() != "document" && path() != "-")
      {
        s += child->render(indent + 1, indent_text, maxlen, current_newline_count);
      }
      else
      {
        s += child->render(indent, indent_text, maxlen, current_newline_count);
      }
    }

    if (path() != "document" && path() != "-")
    {
      wasp::HITNodeView term_node = _hnv.first_child_by_name("term");

      std::string term_data = !term_node.is_null() ? term_node.data() : "[]";

      if (!term_node.is_null())
      {
        append_blank_lines(s, term_node.line(), parent_newline_count);
      }

      s += "\n" + strRepeat(indent_text, indent) + term_data;
    }

    if (indent == 0 &&
        ((root() == this && s[0] == '\n') || (parent() && parent()->children()[0] == this)))
      s = s.substr(1);

    return s;
  }

  // append_blank_lines renders blank lines before sections, fields, and
  // comments by comparing the number of lines already rendered with the
  // line number of the next non-blank returning the gathered line count
  int append_blank_lines(std::string & render_out, int next_print, int start_line = 0)
  {
    int line_tally = start_line + std::count(render_out.begin(), render_out.end(), '\n');
    next_print--;

    if (line_tally < next_print)
    {
      int line_delta = next_print - line_tally;

      render_out += strRepeat("\n", line_delta);

      line_tally += line_delta;
    }

    return line_tally;
  }

  /// clone returns a complete (deep) copy of this node. the caller will be responsible for
  /// managing the memory/deallocation of the returned clone node.
  virtual Node * clone(bool absolute_path = false) override
  {
    auto node = new Section(_dhi, _hnv);

    for (auto child : children())
    {
      node->addChild(child->clone());
    }

    if (absolute_path)
    {
      node->set_override_path(fullpath());
    }

    return node;
  }

  void clearLegacyMarkers()
  {
    if (_hnv.child_count() > 3 && _hnv.child_at(2).type() == wasp::DECL &&
        _hnv.child_at(1).is_leaf())
    {
      _hnv.child_at(1).set_data("");
    }

    auto term_node = _hnv.first_child_by_name("term");

    if (!term_node.is_null() && term_node.is_leaf())
    {
      term_node.set_data("[]");
    }
  }
};

/// Comment represents an in-file comment (i.e. "# some comment text...")
class Comment : public Node
{
public:
  /// constructs a Node from wasp interpreter shared pointer and nodeview
  Comment(std::shared_ptr<wasp::DefaultHITInterpreter> dhi, wasp::HITNodeView hnv)
    : Node(dhi, hnv), _isinline(false)
  {
  }

  /// constructs a Node from comment text - is_inline is currently unused
  Comment(const std::string & text, bool is_inline = false)
    : Node(text, is_inline), _isinline(is_inline)
  {
  }

  /// render builds text with hit syntax that is equivalent to this comment.
  virtual std::string render(int indent = 0,
                             const std::string & indent_text = "  ",
                             int maxlen = 0,
                             int parent_newline_count = 0) override
  {
    (void)maxlen;               // suppress unused variable warning
    (void)parent_newline_count; // suppress unused variable warning

    std::string comment_text = _hnv.data();

    if (_isinline)
    {
      return " " + comment_text;
    }

    std::string s = "\n";

    s += strRepeat(indent_text, indent) + comment_text;

    return s;
  }

  /// clone returns a complete (deep) copy of this node. the caller will be responsible for
  /// managing the memory/deallocation of the returned clone node.
  virtual Node * clone(bool absolute_path = false) override
  {
    auto node = new Comment(_dhi, _hnv);

    if (absolute_path)
    {
      node->set_override_path(fullpath());
    }

    return node;
  }

  /// setText is a way to overwrite and set the comment's text
  void setText(const std::string & text)
  {
    if (text != _hnv.data() && _hnv.is_leaf())
    {
      _hnv.set_data(text.c_str());
    }
  }

private:
  bool _isinline;
};

/// build_hit_tree recursively constructs the hit node tree at the end of the
/// parse method using the root of the data processed by the wasp interpreter
inline void
build_hit_tree(std::shared_ptr<wasp::DefaultHITInterpreter> interpreter,
               wasp::HITNodeView hnv_parent,
               Node * hit_parent)
{
  if (hnv_parent.is_null())
  {
    return;
  }

  for (std::size_t i = 0, count = hnv_parent.child_count(); i < count; i++)
  {
    // wasp nodeview child will be stored in node if one is created and added

    auto hnv_child = hnv_parent.child_at(i);

    // create and add comment node as terminal leaf but do not recurse deeper

    if (hnv_child.type() == wasp::COMMENT)
    {
      auto hit_child = new Comment(interpreter, hnv_child);

      hit_parent->addChild(hit_child);
    }

    // create and add field node as a combined leaf but do not recurse deeper

    else if (hnv_child.type() == wasp::KEYED_VALUE || hnv_child.type() == wasp::ARRAY)
    {
      auto hit_child = new Field(interpreter, hnv_child);

      hit_parent->addChild(hit_child);
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
          build_hit_tree(interpreter, hnv_child, hit_child);

          explode_section_found = true;
        }
      }

      // normal or absent section so create and add node then recurse with it

      if (!explode_section_found)
      {
        auto hit_child = new Section(interpreter, hnv_child);

        hit_parent->addChild(hit_child);

        build_hit_tree(interpreter, hnv_child, hit_child);
      }
    }
  }
}

/// parse is *the* function in the hit namespace. it takes the given hit input text and
/// parses and builds a hit tree returning the root node. it throws an exception if input
/// contains any invalid hit syntax. fname is label given as a convenience (and can be any
/// string) used to prefix any error messages generated during the parsing process. the caller
/// accepts ownership of the returned root node and is responsible for destructing it.
inline Node *
parse(const std::string & fname, const std::string & input)
{
  std::stringstream input_errors;

  std::stringstream input_stream(input);

  std::shared_ptr<wasp::DefaultHITInterpreter> interpreter =
      std::make_shared<wasp::DefaultHITInterpreter>(input_errors);

  if (!interpreter->parseStream(input_stream, fname))
  {
    throw ParseError(input_errors.str());
  }

  std::unique_ptr<Node> hit_root(new Section(interpreter, interpreter->root()));

  build_hit_tree(interpreter, interpreter->root(), hit_root.get());

  return hit_root.release();
}

/// parses the file checking for errors but does not return any node tree.
inline void
check(const std::string & fname, const std::string & input)
{
  delete parse(fname, input);
}

// MergeFieldWalker is used as part of the process of merging two parsed hit node trees.
class MergeFieldWalker : public Walker
{
public:
  MergeFieldWalker(Node * orig) : _orig(orig) {}

  void walk(const std::string & fullpath, const std::string & /*nodepath*/, Node * n) override
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
  Node * _orig;
};

/// merges the hit tree under from into the hit tree under into. if a node (identified by
/// its full path) is present in both trees, the one in from overwrites the one in into. nodes in
/// from but not present in into are cloned. the from tree remains unmodified. line numbers for
/// cloned nodes remain the same as they were in the original from tree - referring to the input
/// that was parsed to create the from tree. in general you should only merge already-exploded
/// node trees.
inline void
merge(Node * from, Node * into)
{
  MergeFieldWalker fw(into);
  MergeSectionWalker sw(into);
  from->walk(&fw);
  from->walk(&sw);
}

/// all explode logic that expands shorthand path notation into sections is
/// handled by a combination of the wasp interpreter and the build_hit_tree
// so this method is a no-op and is temporarily here for interface purposes
inline Node *
explode(Node * n)
{
  return n->root();
}

/// split breaks input into a vector treating whitespace as a delimiter.
/// Consecutive whitespace characters are treated as a single delimiter.
inline std::vector<std::string>
split(const std::string & input)
{
  std::istringstream buffer(input);
  std::vector<std::string> ret((std::istream_iterator<std::string>(buffer)),
                               std::istream_iterator<std::string>());
  return ret;
}

/// lower converts all characters in str to their lower-case versions.
inline std::string
lower(const std::string & str)
{
  std::string l = str;
  std::transform(l.begin(), l.end(), l.begin(), ::tolower);
  return l;
}

/// trim removes consecutive whitespace characters from the beginning and end of str.
inline std::string
trim(const std::string & str)
{
  size_t first = str.find_first_not_of(" \r\n\t");
  if (std::string::npos == first)
    return str;
  size_t last = str.find_last_not_of(" \r\n\t");
  return str.substr(first, (last - first + 1));
}

/// toBool converts the given val to a boolean value which is stored in dst. it returns true if
/// val was successfully converted to a boolean and returns false otherwise.
inline bool
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

/// returns the type of quoting used on string s (i.e. " or ') or an empty string otherwise.
inline std::string
quoteChar(const std::string & s)
{
  if (s[0] == '\'')
    return "'";
  else if (s[0] == '"')
    return "\"";
  return "";
}

/// strRepeat returns a string of s repeated n times.
inline std::string
strRepeat(const std::string & s, int n)
{
  std::string rs;
  for (int i = 0; i < n; i++)
    rs += s;
  return rs;
}

/// pathNorm returns the canonical, normalized version of the given hit path. it removes
/// consecutive slashes and leading './' among other things.
inline std::string
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

/// pathJoin a joined version of the given hit (relative) paths as single hit path.
inline std::string
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

/// matches returns true if s matches the given regex pattern.
inline bool
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

/// Formatter is used to automatically format hit syntax/input to be uniform in a specified style.
/// After creating a formatter object and configuring it as desired by modifying/calling its members
/// you use the "format" function to format hit text as desired.
class Formatter : public Walker
{
public:
  /// constructs a formatter that formats hit text in a canonical default style.
  Formatter() : canonical_section_markers(true), line_length(100), indent_string("  ") {}

  /// constructs a formatter that formats hit text according to the configuration parameters
  /// specified in hit_config - which is the text of a hit document of the following form:
  ///
  ///     [format]
  ///       # these parameters set the correspondingly named Formatter member variables. see them
  ///       # for detailed descriptions of what they do.
  ///       indent_string = "  "
  ///       line_length = 100
  ///       canonical_section_markers = true
  ///
  ///       # this section specifies parameter/section sorgin order as provided by the formatter's
  ///       # addPattern member function. the content for this section mirrors the structure of
  ///       # the hit file being formatted. each section name is a regex (limited to valid hit
  ///       # identifier characters). the fields and subsections within each section specify an
  ///       # order; any field values are ignored. see the docs for that function for more details.
  ///       [sorting]
  ///         [foo]        # section 'foo' goes first (before other sections)
  ///           bar = bla  # field 'bar' (in the foo section) goes first
  ///           ** = bla   # double glob is placeholder for unordered fields/sections
  ///           baz = bla  # field 'baz' goes last
  ///         []
  ///         [.*]
  ///           [.*]
  ///             first = bla # fields named 'first' at double nested level go first
  ///           []
  ///         []
  ///       []
  ///     []
  ///
  /// all fields are optional and the sorting section is also optional. if the sorting section
  /// is present, you can have as many patterns as you like, but each pattern section must have
  /// one set of "section" and "order" fields.
  Formatter(const std::string & fname, const std::string & hit_config)
    : canonical_section_markers(true), line_length(100), indent_string("  ")
  {
    std::unique_ptr<wasp_hit::Node> root(wasp_hit::parse(fname, hit_config));
    if (root->find("format/indent_string"))
      indent_string = root->param<std::string>("format/indent_string");
    if (root->find("format/line_length"))
      line_length = root->param<int>("format/line_length");
    if (root->find("format/canonical_section_markers"))
      canonical_section_markers = root->param<bool>("format/canonical_section_markers");
    if (root->find("format/sorting"))
      walkPatternConfig("", root->find("format/sorting"));
  }

  /// formats the given input hit text (using fname to report better syntax errors) and returns
  /// the text formatted as specified by the formatter's configuration.
  std::string format(const std::string & fname, const std::string & input)
  {
    std::unique_ptr<wasp_hit::Node> root(wasp_hit::parse(fname, input));
    format(root.get());
    return root->render(0, indent_string, line_length);
  }

  void format(wasp_hit::Node * root)
  {
    LegacyMarkerRemover lmr;
    if (canonical_section_markers)
      root->walk(&lmr, wasp_hit::NodeType::Section);

    root->walk(this, wasp_hit::NodeType::All);
  }

  /// add a sorting pattern to the formatter. section is a regex that must match a section's
  /// full path (as returned by a section node's fullpath function). order is a list of regexes
  /// that partial match field names identifying the order of fields for sections that match the
  /// section regex. fields that don't match any order regexes are remain unmoved/ignored. you can
  /// include a "**" entry in the order vector to indicate a glob of unordered parameters. this
  /// allows a specifying an ordered set of parameters for the front of the section as well as an
  /// ordered set of parameters at the back of the section with all parameters not matching any
  /// order regex being placed in their original order in place of the "**".
  void addPattern(const std::string & section, const std::vector<std::string> & order)
  {
    _patterns.push_back({section, order});
  }

  /// walk implements the wasp_hit::Walker interface and should generally not be called.
  void walk(const std::string & fullpath, const std::string & /*nodepath*/, Node * n) override
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

  /// indicates whether or not to convert legacy leading "[./section_name]" in section headers and
  /// closing "[../]" in section footers to the canonical "[section_name]" and "[]" respectively.
  /// if true, the canonical (non-legacy) format is used.
  bool canonical_section_markers;
  /// the maximum length of a line before it will be automatically be broken and reflowed into
  /// multiple lines. note that this only currently applies to string-literals (e.g. not comments,
  /// integers, section headers, etc.)
  int line_length;
  /// the text used to represent a single level of nesting indentation. it must be comprised of
  /// only whitespace characters.
  std::string indent_string;

private:
  struct Pattern
  {
    std::string regex;
    std::vector<std::string> order;
  };

  void walkPatternConfig(const std::string & prefix, Node * n)
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

  void sortGroup(const std::vector<Node *> & nodes,
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

  std::vector<Pattern> _patterns;
};

class GatherParamWalker : public Walker
{
public:
  typedef std::map<std::string, wasp_hit::Node *> ParamMap;

  GatherParamWalker(ParamMap & map) : _map(map) {}

  void
  walk(const std::string & fullpath, const std::string & /*nodepath*/, wasp_hit::Node * n) override
  {
    if (n->type() == wasp_hit::NodeType::Field)
      _map[fullpath] = n;
  }

private:
  ParamMap & _map;
};

class RemoveParamWalker : public Walker
{
public:
  RemoveParamWalker(const GatherParamWalker::ParamMap & map) : _map(map) {}

  void walk(const std::string & /*fullpath*/,
            const std::string & /*nodepath*/,
            wasp_hit::Node * n) override
  {
    auto children = n->children();
    for (auto child : children)
    {
      auto it = _map.find(child->fullpath());
      if (it != _map.end() && it->second->strVal() == child->strVal())
        delete child;
    }
  }

  NodeType nodeType() override { return NodeType::Section; }

private:
  const GatherParamWalker::ParamMap & _map;
};

class RemoveEmptySectionWalker : public Walker
{
public:
  RemoveEmptySectionWalker() {}

  void walk(const std::string & /*fullpath*/,
            const std::string & /*nodepath*/,
            wasp_hit::Node * n) override
  {
    auto children = n->children(NodeType::Section);
    for (auto child : children)
    {
      std::size_t non_blank = 0;
      for (auto gchild : child->children())
        if (gchild->type() != NodeType::Blank && gchild->type() != NodeType::Comment)
          non_blank++;
      if (non_blank == 0)
        delete child;
    }
  }

  NodeType nodeType() override { return NodeType::Section; }

  TraversalOrder traversalOrder() override { return TraversalOrder::AfterChildren; }
};

} // namespace wasp_hit

#endif
