//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JsonInputFileFormatter.h"
#include "MooseUtils.h"

#include <vector>

JsonInputFileFormatter::JsonInputFileFormatter() : _spaces(2), _level(0) {}

std::string
JsonInputFileFormatter::toString(const nlohmann::json & root)
{
  _stream.clear();
  _stream.str("");
  for (auto && el : root["blocks"].items())
    addBlock(el.key(), el.value(), true);
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

  _stream << std::flush;
}

void
JsonInputFileFormatter::addBlock(const std::string & name,
                                 const nlohmann::json & block,
                                 bool toplevel)
{
  addLine("");
  if (toplevel)
    addLine("[" + name + "]");
  else
    addLine("[./" + name + "]");

  _level++;
  std::string desc = block.contains("description") ? nlohmann::to_string(block["description"]) : "";
  if (!desc.empty())
    addLine("", 0, desc);

  if (block.contains("parameters"))
    addParameters(block["parameters"]);

  if (block.contains("actions"))
  {
    // there could be duplicate parameters across actions, last one wins
    nlohmann::json all_params;
    auto & actions = block["actions"];
    for (auto && el : actions.items())
    {
      auto & params = el.value()["parameters"];
      for (auto && param_el : params.items())
        all_params[param_el.key()] = param_el.value();
    }
    addParameters(all_params);
  }

  if (block.contains("star"))
    addBlock("*", block["star"]);

  addTypes("subblock_types", block);
  addTypes("types", block);

  if (block.contains("subblocks"))
  {
    auto & subblocks = block["subblocks"];
    if (!subblocks.is_null())
      for (auto && el : subblocks.items())
        addBlock(el.key(), el.value());
  }

  _level--;
  if (toplevel)
    addLine("[]");
  else
    addLine("[../]");
}

void
JsonInputFileFormatter::addTypes(const std::string & key, const nlohmann::json & block)
{
  if (!block.contains(key))
    return;
  auto & types = block[key];
  if (types.is_null())
    return;

  addLine("");
  addLine("[./<types>]");
  _level++;
  for (auto && el : types.items())
    addBlock("<" + el.key() + ">", el.value());
  _level--;
  addLine("[../]");
}

void
JsonInputFileFormatter::addParameters(const nlohmann::json & params)
{
  size_t max_name = 0;
  for (auto & el : params.items())
    if (el.key().size() > max_name)
      max_name = el.key().size();

  size_t max_len = 0;
  std::map<std::string, std::string> lines;
  for (auto && el : params.items())
  {
    auto & name = el.key();
    auto & param = el.value();
    auto def =
        param.contains("default") ? MooseUtils::trim(nlohmann::to_string(param["default"])) : "";
    if (!def.empty())
      def = def.substr(1, def.size() - 2);
    if (def.find(' ') != std::string::npos)
      def = "'" + def + "'";
    std::string indent(max_name - name.size(), ' ');
    std::string required;
    if (param["required"])
      required = "(required)";
    if (def.size() == 0 && required.size() == 0)
      def = "(no_default)";
    std::string l = name + indent + " = " + def + required;
    if (l.size() > max_len)
      max_len = l.size();
    lines[name] = l;
  }
  for (auto & el : params.items())
  {
    auto & name = el.key();
    auto & param = el.value();
    auto & l = lines[name];
    auto desc = nlohmann::to_string(param["description"]);
    addLine(l, max_len, desc);

    const auto doc_unit = nlohmann::to_string(param["doc_unit"]);
    if (!doc_unit.empty())
      addLine("", max_len + 1,
              "Unit: " + doc_unit); // a +1 to account for an empty line

    const auto group = nlohmann::to_string(param["group_name"]);
    if (!group.empty())
      addLine("", max_len + 1,
              "Group: " + group); // a +1 to account for an empty line
  }
}
