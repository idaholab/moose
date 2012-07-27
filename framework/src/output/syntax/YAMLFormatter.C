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

#include "YAMLFormatter.h"
#include "Parser.h"
#include "MooseEnum.h"

#include <sstream>
#include <vector>

YAMLFormatter::YAMLFormatter(bool dump_mode) :
    SyntaxTree(true),
    _dump_mode(dump_mode)
{
}

std::string
YAMLFormatter::preamble() const
{
  //important: start and end yaml data delimiters used by python
  return "**START YAML DATA**\n";
}

std::string
YAMLFormatter::postscript() const
{
  return "**END YAML DATA**\n";
}

std::string
YAMLFormatter::printParams(InputParameters &params, short depth, const std::string &search_string, bool &found) const
{
  std::ostringstream oss;
  std::string indent(depth*2, ' ');

  for (InputParameters::iterator iter = params.begin(); iter != params.end(); ++iter)
  {
    std::string name = iter->first;
    // First make sure we want to see this parameter, also block active and type
    if (params.isPrivate(iter->first) || name == "active" || (search_string != "" && search_string != iter->first))
      continue;

    found = true;
    // Block params may be required and will have a doc string
    std::string required = params.isParamRequired(iter->first) ? "Yes" : "No";

    oss << indent << "  - name: " << name << "\n";
    oss << indent << "    required: " << required << "\n";
    oss << indent << "    default: !!str ";

    // Only output default if it has one
    if(params.isParamValid(iter->first))
    {
      //prints the value, which is the default value when dumping the tree
      //because it hasn't been changed

      // remove additional '\n' possibly generated in output (breaks YAML parsing)
      std::ostringstream toss;
      iter->second->print(toss);
      std::string tmp_str = toss.str();
      for(std::string::iterator it=tmp_str.begin(); it!=tmp_str.end(); ++it)
      {
        if ( *it == '\n')
        {
          *it = ' ';
        }
      }
      oss << tmp_str;
    }

    std::string doc = params.getDocString(iter->first);
    Parser::escape(doc);
    // Print the type
    oss << "\n" << indent << "    cpp_type: " << params.type(iter->first);

    InputParameters::Parameter<MooseEnum> * enum_type = dynamic_cast<InputParameters::Parameter<MooseEnum>*>(iter->second);
    if (enum_type)
      oss << "\n" << indent << "    options: " << enum_type->get().getRawNamesNoCommas();

    oss << "\n" << indent << "    description: |\n      " << indent
         << doc << "\n";
  }

  return oss.str();
}

std::string
YAMLFormatter::preTraverse(short depth) const
{
  std::string indent(depth*2, ' ');

  return indent + "  subblocks:\n";
}


std::string
YAMLFormatter::printBlockOpen(const std::string &name, short depth, const std::string &type) const
{
  std::ostringstream oss;
  std::string indent(depth*2, ' ');

  oss << indent << "- name: " << (name == "*" ? type : name) << "\n";
  oss << indent << "  description: !!str\n";
  oss << indent << "  type: " << type << "\n";
  oss << indent << "  parameters:\n";

  return oss.str();
}

std::string
YAMLFormatter::printBlockClose(const std::string &/*name*/, short /*depth*/) const
{
  return std::string();
}
