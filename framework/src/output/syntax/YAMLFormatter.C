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
YAMLFormatter::printParams(const std::string &prefix, InputParameters &params, short depth, const std::string &search_string, bool &found)
{
  std::ostringstream oss;
  std::string indent(depth*2, ' ');

  for (InputParameters::iterator iter = params.begin(); iter != params.end(); ++iter)
  {
    std::string name = iter->first;
    // First make sure we want to see this parameter, also block active and type
    if (params.isPrivate(iter->first) || name == "active" || (search_string != "" && search_string != iter->first) || haveSeenIt(prefix, iter->first))
      continue;

    found = true;

    // Mark it as "seen"
    seenIt(prefix, iter->first);

    // Block params may be required and will have a doc string
    std::string required = params.isParamRequired(iter->first) ? "Yes" : "No";

    oss << indent << "  - name: " << name << "\n";
    oss << indent << "    required: " << required << "\n";
    oss << indent << "    default: !!str ";

    // Only output default if it has one
    if (params.isParamValid(iter->first))
    {
      //prints the value, which is the default value when dumping the tree
      //because it hasn't been changed

      // Output stream, performing special operations for writing objects such as Points and RealVectorValues
      std::ostringstream toss;
      buildOutputString(toss, iter);

      // remove additional '\n' possibly generated in output (breaks YAML parsing)
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
    MooseUtils::escape(doc);
    // Print the type
    oss << "\n" << indent << "    cpp_type: " << params.type(iter->first)
        << "\n" << indent << "    group_name: " << params.getGroupName(iter->first);

    {
      InputParameters::Parameter<MooseEnum> * enum_type = dynamic_cast<InputParameters::Parameter<MooseEnum>*>(iter->second);
      if (enum_type)
        oss << "\n" << indent << "    options: " << enum_type->get().getRawNamesNoCommas();
    }
    {
      InputParameters::Parameter<std::vector<MooseEnum> > * enum_type = dynamic_cast<InputParameters::Parameter<std::vector<MooseEnum> >*>(iter->second);
      if (enum_type)
        oss << "\n" << indent << "    options: " << (enum_type->get())[0].getRawNamesNoCommas();
    }

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

void
YAMLFormatter::buildOutputString(std::ostringstream & output, const InputParameters::iterator iter)
{

  // Account for Point
  InputParameters::Parameter<Point> * ptr0 = dynamic_cast<InputParameters::Parameter<Point>*>(iter->second);

  // Account for RealVectorValues
  InputParameters::Parameter<RealVectorValue> * ptr1  = dynamic_cast<InputParameters::Parameter<RealVectorValue>*>(iter->second);

  // Output the Point components
  if (ptr0)
  {
    output << ptr0->get().operator()(0) << " " << ptr0->get().operator()(1) << " " << ptr0->get().operator()(2);
  }

  // Output the RealVectorValue components
  else if (ptr1)
  {
    output << ptr1->get().operator()(0) << " " << ptr1->get().operator()(1) << " " << ptr1->get().operator()(2);
  }

  // General case, call the print operator
  else
  {
    iter->second->print(output);
  }

}
