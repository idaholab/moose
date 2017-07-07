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

#include "JsonSyntaxTree.h"

// MOOSE includes
#include "MooseEnum.h"
#include "MultiMooseEnum.h"
#include "Parser.h"
#include "pcrecpp.h"

// C++ includes
#include <algorithm>
#include <cctype>

JsonSyntaxTree::JsonSyntaxTree(const std::string & search_string) : _search(search_string) {}

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

moosecontrib::Json::Value &
JsonSyntaxTree::getJson(const std::string & path)
{
  auto paths = splitPath(path);
  mooseAssert(paths.size() > 0, "path is empty");
  moosecontrib::Json::Value * next = &_root["blocks"][paths[0]];

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

moosecontrib::Json::Value &
JsonSyntaxTree::getJson(const std::string & parent, const std::string & path, bool is_type)
{
  if (parent.empty())
  {
    auto & j = getJson(path);
    if (path.back() == '*' && !j.isMember("subblock_types"))
      j["subblock_types"] = moosecontrib::Json::Value();
    else if (path.back() != '*' && !j.isMember("types"))
      j["types"] = moosecontrib::Json::Value();
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
JsonSyntaxTree::setParams(InputParameters * params,
                          bool search_match,
                          moosecontrib::Json::Value & all_params)
{
  size_t count = 0;
  for (auto & iter : *params)
  {
    // Make sure we want to see this parameter
    bool param_match = !_search.empty() && MooseUtils::wildCardMatch(iter.first, _search);
    if (params->isPrivate(iter.first) || (!_search.empty() && !search_match && !param_match))
      continue;

    ++count;
    moosecontrib::Json::Value param_json;

    param_json["required"] = params->isParamRequired(iter.first);

    // Only output default if it has one
    if (params->isParamValid(iter.first))
      param_json["default"] = buildOutputString(iter);
    else if (params->hasDefaultCoupledValue(iter.first))
      param_json["default"] = params->defaultCoupledValue(iter.first);

    param_json["options"] = buildOptions(iter);
    auto reserved_values = params->reservedValues(iter.first);
    for (const auto & reserved : reserved_values)
      param_json["reserved_values"].append(reserved);

    std::string t = prettyCppType(params->type(iter.first));
    param_json["cpp_type"] = t;
    param_json["basic_type"] = basicCppType(t);
    param_json["group_name"] = params->getGroupName(iter.first);
    param_json["name"] = iter.first;

    std::string doc = params->getDocString(iter.first);
    MooseUtils::escape(doc);
    param_json["description"] = doc;
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
    auto params = validParams<Action>();
    moosecontrib::Json::Value jparams;
    setParams(&params, true, jparams);
    _root["global"]["parameters"] = jparams;
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

  moosecontrib::Json::Value all_params;
  bool search_match = !_search.empty() && (MooseUtils::wildCardMatch(path, _search) ||
                                           MooseUtils::wildCardMatch(action, _search) ||
                                           MooseUtils::wildCardMatch(parent, _search));
  auto count = setParams(params, search_match, all_params);
  if (!_search.empty() && count == 0)
    // no parameters that matched the search string
    return false;

  moosecontrib::Json::Value & json = getJson(parent, path, is_type);

  if (is_action)
  {
    json[action]["parameters"] = all_params;
    json[action]["description"] = params->getClassDescription();
    json[action]["action_path"] = path;
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
JsonSyntaxTree::buildOptions(const std::iterator_traits<InputParameters::iterator>::value_type & p)
{
  std::string options;
  {
    InputParameters::Parameter<MooseEnum> * enum_type =
        dynamic_cast<InputParameters::Parameter<MooseEnum> *>(p.second);
    if (enum_type)
      options = enum_type->get().getRawNames();
  }
  {
    InputParameters::Parameter<MultiMooseEnum> * enum_type =
        dynamic_cast<InputParameters::Parameter<MultiMooseEnum> *>(p.second);
    if (enum_type)
      options = enum_type->get().getRawNames();
  }
  {
    InputParameters::Parameter<std::vector<MooseEnum>> * enum_type =
        dynamic_cast<InputParameters::Parameter<std::vector<MooseEnum>> *>(p.second);
    if (enum_type)
      options = (enum_type->get())[0].getRawNames();
  }
  return options;
}

std::string
JsonSyntaxTree::buildOutputString(
    const std::iterator_traits<InputParameters::iterator>::value_type & p)
{
  // Account for Point
  std::stringstream str;
  InputParameters::Parameter<Point> * ptr0 =
      dynamic_cast<InputParameters::Parameter<Point> *>(p.second);

  // Account for RealVectorValues
  InputParameters::Parameter<RealVectorValue> * ptr1 =
      dynamic_cast<InputParameters::Parameter<RealVectorValue> *>(p.second);

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
    p.second->print(str);

  // remove additional '\n' possibly generated in output (breaks JSON parsing)
  std::string tmp_str = str.str();
  for (auto & ch : tmp_str)
    if (ch == '\n')
      ch = ' ';
  return tmp_str;
}

void
JsonSyntaxTree::addSyntaxType(const std::string & path, const std::string type)
{
  if (MooseUtils::wildCardMatch(path, _search))
  {
    auto & j = getJson(path);
    j["associated_types"].append(type);
  }
}

void
JsonSyntaxTree::addActionTask(const std::string & path,
                              const std::string & action,
                              const std::string & task_name,
                              const FileLineInfo & lineinfo)
{
  moosecontrib::Json::Value & json = getJson("", path, false);
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

std::string
JsonSyntaxTree::prettyCppType(const std::string & cpp_type)
{
  // On mac many of the std:: classes are inline namespaced with __1
  // On linux std::string can be inline namespaced with __cxx11
  std::string s = cpp_type;
  pcrecpp::RE("std::__\\w+::").GlobalReplace("std::", &s);
  // It would be nice if std::string actually looked normal
  pcrecpp::RE("\\s*std::basic_string<char, std::char_traits<char>, std::allocator<char> >\\s*")
      .GlobalReplace("std::string", &s);
  // It would be nice if std::vector looked normal
  pcrecpp::RE r("std::vector<([[:print:]]+),\\s?std::allocator<\\s?\\1\\s?>\\s?>");
  r.GlobalReplace("std::vector<\\1>", &s);
  // Do it again for nested vectors
  r.GlobalReplace("std::vector<\\1>", &s);
  return s;
}
