//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JsonSyntaxTree.h"

// MOOSE includes
#include "MooseEnum.h"
#include "MultiMooseEnum.h"
#include "ExecFlagEnum.h"
#include "Parser.h"
#include "pcrecpp.h"
#include "Action.h"
#include "AppFactory.h"
#include "Registry.h"
#include "MooseUtils.h"

#include "libmesh/vector_value.h"

// C++ includes
#include <algorithm>
#include <cctype>

JsonSyntaxTree::JsonSyntaxTree(const std::string & search_string) : _search(search_string)
{
  // Registry holds a map with labels (ie MooseApp) as keys and a vector of RegistryEntry
  // as values. We need the reverse map: given an action or object name then get the label.
  auto & actmap = Registry::allActions();
  for (auto & entry : actmap)
    for (auto & act : entry.second)
      _action_label_map[act._classname] = std::make_pair(entry.first, act._file);

  auto & objmap = Registry::allObjects();
  for (auto & entry : objmap)
    for (auto & obj : entry.second)
    {
      std::string name = obj._name;
      if (name.empty())
        name = obj._alias;
      if (name.empty())
        name = obj._classname;

      // Skip <JACOBIAN> instances and remove <RESIDUAL> from name
      if (name.find("<JACOBIAN>") == std::string::npos)
      {
        name = name.substr(0, name.find("<RESIDUAL>"));
        _object_label_map[name] = std::make_pair(entry.first, obj._file);
      }
    }
}

std::vector<std::string>
JsonSyntaxTree::splitPath(const std::string & path)
{
  std::string s;
  std::istringstream f(path);
  std::vector<std::string> paths;
  while (std::getline(f, s, '/'))
    if (s.size() > 0)
      paths.push_back(s);
  return paths;
}

nlohmann::json &
JsonSyntaxTree::getJson(const std::string & path)
{
  auto paths = splitPath(path);
  mooseAssert(paths.size() > 0, "path is empty");
  auto * next = &(_root["blocks"][paths[0]]);

  for (auto pit = paths.begin() + 1; pit != paths.end(); ++pit)
  {
    if (*pit == "*")
      // It has an action syntax as a parent
      next = &(*next)["star"];
    else if (*pit == "<type>")
      next = &(*next)["types"];
    else
      next = &(*next)["subblocks"][*pit];
  }
  return *next;
}

nlohmann::json &
JsonSyntaxTree::getJson(const std::string & parent, const std::string & path, bool is_type)
{
  if (parent.empty())
  {
    auto & j = getJson(path);
    if (path.back() == '*' && !j.contains("subblock_types"))
      j["subblock_types"] = nlohmann::json();
    else if (path.back() != '*' && !j.contains("types"))
      j["types"] = nlohmann::json();
    return j["actions"];
  }

  auto & parent_json = getJson(parent);
  auto paths = splitPath(path);
  std::string key = "subblock_types";
  if (is_type)
    key = "types";
  auto & val = parent_json[key][paths.back()];
  return val;
}

size_t
JsonSyntaxTree::setParams(InputParameters * params, bool search_match, nlohmann::json & all_params)
{
  size_t count = 0;
  for (auto & iter : *params)
  {
    // Make sure we want to see this parameter
    bool param_match = !_search.empty() && MooseUtils::wildCardMatch(iter.first, _search);
    if (params->isPrivate(iter.first) || (!_search.empty() && !search_match && !param_match))
      continue;

    ++count;
    nlohmann::json param_json;

    param_json["required"] = params->isParamRequired(iter.first);

    // Only output default if it has one
    if (params->isParamValid(iter.first))
      param_json["default"] = buildOutputString(iter);
    else if (params->hasDefaultCoupledValue(iter.first))
      param_json["default"] = params->defaultCoupledValue(iter.first);

    bool out_of_range_allowed = false;
    std::map<MooseEnumItem, std::string> docs;
    param_json["options"] = buildOptions(iter, out_of_range_allowed, docs);
    if (!nlohmann::to_string(param_json["options"]).empty())
    {
      param_json["out_of_range_allowed"] = out_of_range_allowed;
      if (!docs.empty())
      {
        nlohmann::json jdocs;
        for (const auto & doc : docs)
          jdocs[doc.first.name()] = doc.second;
        param_json["option_docs"] = jdocs;
      }
    }
    auto reserved_values = params->reservedValues(iter.first);
    for (const auto & reserved : reserved_values)
      param_json["reserved_values"].push_back(reserved);

    std::string t = MooseUtils::prettyCppType(params->type(iter.first));
    param_json["cpp_type"] = t;
    param_json["basic_type"] = basicCppType(t);
    param_json["group_name"] = params->getGroupName(iter.first);
    param_json["name"] = iter.first;

    std::string doc = params->getDocString(iter.first);
    MooseUtils::escape(doc);
    param_json["description"] = doc;
    param_json["controllable"] = params->isControllable(iter.first);
    param_json["deprecated"] = params->isParamDeprecated(iter.first);
    all_params[iter.first] = param_json;
  }
  return count;
}

void
JsonSyntaxTree::addGlobal()
{
  // If they are doing a search they probably don't want to see this
  if (_search.empty())
  {
    auto params = Action::validParams();
    nlohmann::json jparams;
    setParams(&params, true, jparams);
    _root["global"]["parameters"] = jparams;

    // Just create a list of registered app names
    nlohmann::json apps;
    auto & factory = AppFactory::instance();
    for (auto app = factory.registeredObjectsBegin(); app != factory.registeredObjectsEnd(); ++app)
      apps.push_back(app->first);

    _root["global"]["registered_apps"] = apps;
  }
}

bool
JsonSyntaxTree::addParameters(const std::string & parent,
                              const std::string & path,
                              bool is_type,
                              const std::string & action,
                              bool is_action,
                              InputParameters * params,
                              const FileLineInfo & lineinfo,
                              const std::string & classname)
{
  if (action == "EmptyAction")
    return false;

  nlohmann::json all_params;
  bool search_match = !_search.empty() && (MooseUtils::wildCardMatch(path, _search) ||
                                           MooseUtils::wildCardMatch(action, _search) ||
                                           MooseUtils::wildCardMatch(parent, _search));
  auto count = setParams(params, search_match, all_params);
  if (!_search.empty() && count == 0)
    // no parameters that matched the search string
    return false;

  nlohmann::json & json = getJson(parent, path, is_type);

  if (is_action)
  {
    json[action]["parameters"] = all_params;
    json[action]["description"] = params->getClassDescription();
    json[action]["action_path"] = path;
    auto label_pair = getActionLabel(action);
    json[action]["label"] = label_pair.first;
    json[action]["register_file"] = label_pair.second;
    if (lineinfo.isValid())
      json[action]["file_info"][lineinfo.file()] = lineinfo.line();
  }
  else if (params)
  {
    if (params->isParamValid("_moose_base"))
      json["moose_base"] = params->get<std::string>("_moose_base");

    json["parameters"] = all_params;
    json["syntax_path"] = path;
    json["parent_syntax"] = parent;
    json["description"] = params->getClassDescription();
    auto label_pair = getObjectLabel(path);
    json["label"] = label_pair.first;
    json["register_file"] = label_pair.second;
    if (lineinfo.isValid())
    {
      json["file_info"][lineinfo.file()] = lineinfo.line();
      if (!classname.empty())
        json["class"] = classname;
    }
  }
  return true;
}

std::string
JsonSyntaxTree::buildOptions(const std::iterator_traits<InputParameters::iterator>::value_type & p,
                             bool & out_of_range_allowed,
                             std::map<MooseEnumItem, std::string> & docs)
{
  libMesh::Parameters::Value * val = MooseUtils::get(p.second);

  std::string options;
  {
    auto * enum_type = dynamic_cast<InputParameters::Parameter<MooseEnum> *>(val);
    if (enum_type)
    {
      out_of_range_allowed = enum_type->get().isOutOfRangeAllowed();
      options = enum_type->get().getRawNames();
      docs = enum_type->get().getItemDocumentation();
    }
  }
  {
    auto * enum_type = dynamic_cast<InputParameters::Parameter<MultiMooseEnum> *>(val);
    if (enum_type)
    {
      out_of_range_allowed = enum_type->get().isOutOfRangeAllowed();
      options = enum_type->get().getRawNames();
      docs = enum_type->get().getItemDocumentation();
    }
  }
  {
    auto * enum_type = dynamic_cast<InputParameters::Parameter<ExecFlagEnum> *>(val);
    if (enum_type)
    {
      out_of_range_allowed = enum_type->get().isOutOfRangeAllowed();
      options = enum_type->get().getRawNames();
      docs = enum_type->get().getItemDocumentation();
    }
  }
  {
    auto * enum_type = dynamic_cast<InputParameters::Parameter<std::vector<MooseEnum>> *>(val);
    if (enum_type)
    {
      out_of_range_allowed = (enum_type->get())[0].isOutOfRangeAllowed();
      options = (enum_type->get())[0].getRawNames();
      docs = enum_type->get()[0].getItemDocumentation();
    }
  }
  return options;
}

std::string
JsonSyntaxTree::buildOutputString(
    const std::iterator_traits<InputParameters::iterator>::value_type & p)
{
  libMesh::Parameters::Value * val = MooseUtils::get(p.second);

  // Account for Point
  std::stringstream str;
  InputParameters::Parameter<Point> * ptr0 = dynamic_cast<InputParameters::Parameter<Point> *>(val);

  // Account for RealVectorValues
  InputParameters::Parameter<RealVectorValue> * ptr1 =
      dynamic_cast<InputParameters::Parameter<RealVectorValue> *>(val);

  // Output the Point components
  if (ptr0)
    str << ptr0->get().operator()(0) << " " << ptr0->get().operator()(1) << " "
        << ptr0->get().operator()(2);

  // Output the RealVectorValue components
  else if (ptr1)
    str << ptr1->get().operator()(0) << " " << ptr1->get().operator()(1) << " "
        << ptr1->get().operator()(2);

  // General case, call the print operator
  else
    val->print(str);

  // remove additional '\n' possibly generated in output (breaks JSON parsing)
  std::string tmp_str = str.str();
  for (auto & ch : tmp_str)
    if (ch == '\n')
      ch = ' ';

  return tmp_str.substr(0, tmp_str.find("<RESIDUAL>"));
}

void
JsonSyntaxTree::addSyntaxType(const std::string & path, const std::string type)
{
  if (MooseUtils::wildCardMatch(path, _search))
  {
    auto & j = getJson(path);
    j["associated_types"].push_back(type);
  }
  // If they are doing a search they probably don't want to see this
  if (_search.empty())
  {
    _root["global"]["associated_types"][type].push_back(path);
  }
}

void
JsonSyntaxTree::addActionTask(const std::string & path,
                              const std::string & action,
                              const std::string & task_name,
                              const FileLineInfo & lineinfo)
{
  nlohmann::json & json = getJson("", path, false);
  if (lineinfo.isValid())
    json[action]["tasks"][task_name]["file_info"][lineinfo.file()] = lineinfo.line();
}

std::string
JsonSyntaxTree::basicCppType(const std::string & cpp_type)
{
  std::string s = "String";
  if (cpp_type.find("std::vector") != std::string::npos ||
      cpp_type.find("libMesh::VectorValue") != std::string::npos ||
      cpp_type.find("libMesh::TensorValue") != std::string::npos)
  {
    // Get the template type and use its basic type for the array type
    pcrecpp::RE r("^[^<]+<\\s*(.*)\\s*>$");
    std::string t;
    r.FullMatch(cpp_type, &t);
    s = "Array:" + basicCppType(t);
  }
  else if (cpp_type.find("MultiMooseEnum") != std::string::npos ||
           cpp_type.find("ExecFlagEnum") != std::string::npos ||
           cpp_type.find("VectorPostprocessorName") != std::string::npos)
    s = "Array:String";
  else if (cpp_type.find("libMesh::Point") != std::string::npos)
    s = "Array:Real";
  else if (cpp_type == "int" || cpp_type == "unsigned int" || cpp_type == "short" ||
           cpp_type == "unsigned short" || cpp_type == "char" || cpp_type == "unsigned char" ||
           cpp_type == "long" || cpp_type == "unsigned long")
    s = "Integer";
  else if (cpp_type == "double" || cpp_type == "float")
    s = "Real";
  else if (cpp_type == "bool")
    s = "Boolean";

  return s;
}

std::pair<std::string, std::string>
JsonSyntaxTree::getObjectLabel(const std::string & obj) const
{
  auto paths = splitPath(obj);
  auto it = _object_label_map.find(paths.back());
  if (it != _object_label_map.end())
    return it->second;
  else
    return std::make_pair("", "");
}

std::pair<std::string, std::string>
JsonSyntaxTree::getActionLabel(const std::string & action) const
{
  auto it = _action_label_map.find(action);
  if (it != _action_label_map.end())
    return it->second;
  else
    return std::make_pair("", "");
}
