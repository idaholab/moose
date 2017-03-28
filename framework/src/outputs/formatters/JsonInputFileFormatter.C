/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "JsonInputFileFormatter.h"
#include "MooseUtils.h"

#include <vector>

JsonInputFileFormatter::JsonInputFileFormatter() : _spaces(2), _level(0) {}

std::string
JsonInputFileFormatter::toString(const moosecontrib::Json::Value & root)
{
  _stream.clear();
  _stream.str("");
  for (auto && name : root.getMemberNames())
    addBlock(name, root[name], true);
  return _stream.str();
}

void
JsonInputFileFormatter::addLine(const std::string & line,
                                size_t max_line_len,
                                const std::string & comment)
{
  if (line.empty() && comment.empty())
  {
    _stream << "\n";
    return;
  }

  std::string indent(_level * _spaces, ' ');
  auto doc = MooseUtils::trim(comment);
  if (doc.empty()) // Not comment so just print out the line normally
  {
    _stream << indent << line << "\n";
    return;
  }

  // We have a comment so we need to break it up over multiple lines if necessary
  // and make sure that they all start at the same spot.
  _stream << indent << line;
  std::vector<std::string> elements;

  // if the line is empty we can just start the comment right away
  int extra = 1;
  if (line.empty())
    extra = 0;

  // be careful of really long lines.
  int len = 100 - max_line_len - indent.size();
  if (len < 0)
    len = 20;

  MooseUtils::tokenize(doc, elements, len, " \t");
  std::string first(max_line_len - line.size() + extra, ' ');
  _stream << first << "# " << elements[0] << "\n";
  std::string cindent(max_line_len + indent.size() + extra, ' ');
  for (size_t i = 1; i < elements.size(); ++i)
    _stream << cindent << "# " << elements[i] << "\n";
}

void
JsonInputFileFormatter::addBlock(const std::string & name,
                                 const moosecontrib::Json::Value & block,
                                 bool toplevel)
{
  addLine("");
  if (toplevel)
    addLine("[" + name + "]");
  else
    addLine("[./" + name + "]");

  _level++;
  if (block.isMember("description") && !block["description"].asString().empty())
    addLine("", 0, block["description"].asString());

  if (block.isMember("parameters"))
    addParameters(block["parameters"]);

  if (block.isMember("actions"))
  {
    // there could be duplicate parameters across actions, last one wins
    moosecontrib::Json::Value all_params;
    auto & actions = block["actions"];
    for (auto && name : actions.getMemberNames())
    {
      auto & params = actions[name]["parameters"];
      for (auto && param_name : params.getMemberNames())
        all_params[param_name] = params[param_name];
    }
    addParameters(all_params);
  }

  if (block.isMember("star"))
    addBlock("*", block["star"]);

  addTypes("subblock_types", block);
  addTypes("types", block);

  if (block.isMember("subblocks"))
  {
    auto & subblocks = block["subblocks"];
    if (!subblocks.isNull())
      for (auto && name : subblocks.getMemberNames())
        addBlock(name, subblocks[name]);
  }

  _level--;
  if (toplevel)
    addLine("[]");
  else
    addLine("[../]");
}

void
JsonInputFileFormatter::addTypes(const std::string & key, const moosecontrib::Json::Value & block)
{
  if (!block.isMember(key))
    return;
  auto & types = block[key];
  if (types.isNull())
    return;

  addLine("");
  addLine("[./<types>]");
  _level++;
  for (auto && name : types.getMemberNames())
    addBlock("<" + name + ">", types[name]);
  _level--;
  addLine("[../]");
}

void
JsonInputFileFormatter::addParameters(const moosecontrib::Json::Value & params)
{
  size_t max_name = 0;
  for (auto && name : params.getMemberNames())
    if (name.size() > max_name)
      max_name = name.size();

  size_t max_len = 0;
  std::map<std::string, std::string> lines;
  for (auto && name : params.getMemberNames())
  {
    auto & param = params[name];
    auto def = MooseUtils::trim(param["default"].asString());
    if (def.find(' ') != std::string::npos)
      def = "'" + def + "'";
    std::string indent(max_name - name.size(), ' ');
    std::string required;
    if (param["required"].asString() == "Yes")
      required = "(required)";
    std::string l = name + indent + " = " + def + required;
    if (l.size() > max_len)
      max_len = l.size();
    lines[name] = l;
  }
  for (auto && name : params.getMemberNames())
  {
    auto & param = params[name];
    auto & l = lines[name];
    auto desc = param["description"].asString();
    addLine(l, max_len, desc);

    auto group = param["group_name"].asString();
    if (!group.empty())
      addLine("", max_len + 1, "Group: " + group); // a +1 to account for an empty line
  }
}
