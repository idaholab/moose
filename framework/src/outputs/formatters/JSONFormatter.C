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

#include "JSONFormatter.h"
#include "MooseEnum.h"
#include "Parser.h"

#include <sstream>
#include <vector>

JSONFormatter::JSONFormatter(bool dump_mode)
  : SyntaxTree(true),
    _dump_mode(dump_mode)
{
}

std::string
JSONFormatter::postscript() const
{
  std::stringstream s;
  s << "**START JSON DATA**\n" << _json << "\n**END JSON DATA**\n";
  return s.str();
}

std::vector<std::string>
JSONFormatter::splitPath(const std::string & long_name) const
{
  std::string s;
  std::istringstream f(long_name);
  std::vector<std::string> paths;
  while (std::getline(f, s, '/'))
    if (s.size() > 0)
      paths.push_back(s);
  return paths;
}

Json::Value &
JSONFormatter::getJson(const std::string & full_path)
{
  auto paths = splitPath(full_path);
  Json::Value * next = &_json[paths[0]];
  for (auto pit = paths.begin() + 1; pit != paths.end(); ++pit)
    next = &((*next)[*pit]);
  return *next;
}

std::string
JSONFormatter::printParams(const std::string & prefix, const std::string & fully_qualified_name,
                           InputParameters & params, short /*depth*/, const std::string & search_string, bool & found)
{
  Json::Value all_params;

  for (auto & iter : params)
  {
    std::string name = iter.first;
    // First make sure we want to see this parameter, also block active and type
    if (params.isPrivate(iter.first) || name == "active" || (search_string != "" && search_string != iter.first) || haveSeenIt(prefix, iter.first))
      continue;

    Json::Value param_json;
    found = true;

    // Mark it as "seen"
    seenIt(prefix, iter.first);

    // Block params may be required and will have a doc string
    std::string required = params.isParamRequired(iter.first) ? "Yes" : "No";

    param_json["name"] = name;
    param_json["required"] = required;

    // Only output default if it has one
    if (params.isParamValid(iter.first))
    {
      //prints the value, which is the default value when dumping the tree
      //because it hasn't been changed

      // Output stream, performing special operations for writing objects such as Points and RealVectorValues
      std::ostringstream toss;
      buildOutputString(toss, iter);

      // remove additional '\n' possibly generated in output (breaks JSON parsing)
      std::string tmp_str = toss.str();
      for (auto & ch : tmp_str)
        if (ch == '\n')
          ch = ' ';
      param_json["default"] = tmp_str;
    }
    else if (params.hasDefaultCoupledValue(iter.first))
      param_json["default"] = params.defaultCoupledValue(iter.first);

    param_json["cpp_type"] = params.type(iter.first);
    param_json["group_name"] = params.getGroupName(iter.first);

    {
      InputParameters::Parameter<MooseEnum> * enum_type = dynamic_cast<InputParameters::Parameter<MooseEnum> *>(iter.second);
      if (enum_type)
        param_json["options"] = enum_type->get().getRawNames();
    }
    {
      InputParameters::Parameter<MultiMooseEnum> * enum_type = dynamic_cast<InputParameters::Parameter<MultiMooseEnum> *>(iter.second);
      if (enum_type)
        param_json["options"] = enum_type->get().getRawNames();
    }
    {
      InputParameters::Parameter<std::vector<MooseEnum>> * enum_type = dynamic_cast<InputParameters::Parameter<std::vector<MooseEnum>> *>(iter.second);
      if (enum_type)
        param_json["options"] = (enum_type->get())[0].getRawNames();
    }

    std::string doc = params.getDocString(iter.first);
    MooseUtils::escape(doc);
    param_json["description"] = doc;
    all_params[name] = param_json;
  }
  Json::Value & json = getJson(fully_qualified_name);
  json["full_path"] = fully_qualified_name;
  json["parameters"] = all_params;
  return std::string();
}

std::string
JSONFormatter::printBlockOpen(const std::string & name, short /*depth*/, const std::string & doc)
{
  Json::Value & json = getJson(name);
  std::string docEscaped = doc;
  MooseUtils::escape(docEscaped);
  json["description"] = docEscaped;
  return std::string();
}

std::string
JSONFormatter::printBlockClose(const std::string & /*name*/, short /*depth*/) const
{
  return std::string();
}

void
JSONFormatter::buildOutputString(std::ostringstream & output, const std::iterator_traits<InputParameters::iterator>::value_type & p)
{
  // Account for Point
  InputParameters::Parameter<Point> * ptr0 = dynamic_cast<InputParameters::Parameter<Point> *>(p.second);

  // Account for RealVectorValues
  InputParameters::Parameter<RealVectorValue> * ptr1 = dynamic_cast<InputParameters::Parameter<RealVectorValue> *>(p.second);

  // Output the Point components
  if (ptr0)
    output << ptr0->get().operator()(0) << " " << ptr0->get().operator()(1) << " " << ptr0->get().operator()(2);

  // Output the RealVectorValue components
  else if (ptr1)
    output << ptr1->get().operator()(0) << " " << ptr1->get().operator()(1) << " " << ptr1->get().operator()(2);

  // General case, call the print operator
  else
    p.second->print(output);
}
