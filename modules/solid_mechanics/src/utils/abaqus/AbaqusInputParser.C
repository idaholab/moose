//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusInputParser.h"
#include "MooseError.h"

namespace Abaqus
{

HeaderMap::HeaderMap(std::vector<std::string> fields)
{
  fields.erase(fields.begin());
  for (const auto & field : fields)
  {
    std::vector<std::string> pair;
    MooseUtils::tokenize(field, pair, 2, "=");
    _map[MooseUtils::toLower(MooseUtils::removeExtraWhitespace(pair[0]))] =
        pair.size() == 1 ? "" : MooseUtils::removeExtraWhitespace(pair[1]);
  }
}

bool
HeaderMap::has(const std::string & key) const
{
  return _map.find(MooseUtils::toLower(key)) != _map.end();
}

template <>
bool
HeaderMap::get(const std::string & key) const
{
  const auto it = _map.find(MooseUtils::toLower(key));
  return it != _map.end();
}

std::string
HeaderMap::stringify(const std::string & indent) const
{
  std::string s;
  for (const auto & [key, value] : _map)
    if (value.empty())
      s += indent + key + "\n";
    else
      s += indent + key + " = " + value + "\n";
  return s;
}

/**
 * These sets help the parser distinguish between blocks and options, where
 * options are keyword lines followed by data lines, and blocks are input
 * sections terminated by an `*end xxxx` keyword that can nest other blocks
 * and options.
 */
const static std::set<std::string> abaqus_blocks = {"assembly", "part", "step", "instance"};
const static std::set<std::string> abaqus_options = {
    "boundary",
    "dload",
    "element",
    "elset",
    "heading",
    "initial conditions",
    "node",
    "nset",
    "output",
    "preprint",
    "restart",
    "static",
    "uel property",
    "user element",
};

void
InputParser::parse(std::istream & in)
{
  // load and preprocess entire file
  loadFile(in);

  // build the parse tree
  parseBlockInternal(*this);
}

void
InputParser::loadFile(std::istream & in)
{
  _current_line = 0;

  std::string s, ss;
  while (true)
  {
    if (in.eof())
      break;

    // read line
    std::getline(in, s);
    ss = s;

    // continuation lines
    while (ss.length() > 0 && ss.back() == ',')
    {
      std::getline(in, s);

      // error, expected a continuation line but got EOF
      if (in.eof())
        mooseError("Expected a continuation line but got EOF in Abaqus input file");

      ss += s;
    }

    // skip empty lines and comments (todo: remove spaces)
    if (ss.length() < 2 || ss.substr(0, 2) == "**")
      continue;

    // tokenize and trim
    std::vector<std::string> items;
    MooseUtils::tokenize(ss, items, 1, ",");
    for (auto & item : items)
      item = MooseUtils::removeExtraWhitespace(item);

    _lines.push_back(items);
  }
}

BlockNode
InputParser::parseBlock(const std::string & end, const std::vector<std::string> & head_line)
{
  BlockNode node(head_line);
  parseBlockInternal(node, end);
  return node;
}

void
InputParser::parseBlockInternal(BlockNode & node, const std::string & end)
{
  while (_current_line < _lines.size())
  {
    const auto & line = _lines[_current_line];
    // std::cout << Moose::stringify(line) << '\n';
    if (line[0][0] != '*')
      mooseError("Expected a keyword line in Abaqus input file");
    const auto keyword = MooseUtils::toLower(line[0].substr(1));

    // subblock end
    if (!end.empty() && keyword == "end " + end)
    {
      _current_line++;
      break;
    }

    if (abaqus_blocks.count(keyword))
    {
      _current_line++;
      node._children.push_back(parseBlock(keyword, line));
    }

    else if (abaqus_options.count(keyword))
    {
      _current_line++;
      node._options.push_back(parseOption(line));
    }

    else
      mooseError("Unknown keyword '" + line[0] +
                 "' in Abaqus input file. Add this to abaqus_blocks or abaqus_options in "
                 "InputParser.C to parse it.");
  }
}

OptionNode
InputParser::parseOption(const std::vector<std::string> & head_line)
{
  OptionNode node(head_line);
  while (_current_line < _lines.size())
  {
    const auto & line = _lines[_current_line];
    if (line[0][0] == '*')
      break;
    node._data.push_back(line);
    _current_line++;
  }
  return node;
}

Node::Node(std::vector<std::string> line)
{
  _type = line[0];
  _header = HeaderMap(line);
}

std::string
OptionNode::stringify(const std::string & indent) const
{
  std::string s = indent + "<" + _type + ">\n";
  s += _header.stringify(indent + "  ");
  for (const auto & line : _data)
    s += indent + "    " + Moose::stringify(line) + "\n";
  s += indent + "<>\n";
  return s;
}

std::string
BlockNode::stringify(const std::string & indent) const
{
  std::string s = indent + "[" + _type + "]\n";
  s += _header.stringify(indent + "  ");
  for (const auto & child : _children)
    s += child.stringify(indent + "  ");
  for (const auto & option : _options)
    s += option.stringify(indent + "  ");
  s += indent + "[]\n";
  return s;
}

} // namespace Abaqus
