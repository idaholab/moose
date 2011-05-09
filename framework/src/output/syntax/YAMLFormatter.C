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

#include <sstream>
#include <vector>

YAMLFormatter::YAMLFormatter(std::ostream & out, bool dump_mode)
  :SyntaxFormatterInterface(out, dump_mode)
{
}

void
YAMLFormatter::preamble()
{
  //important: start and end yaml data delimiters used by python
  _out << "**START YAML DATA**\n";
  _out << "  - name: TODO\n";
  _out << "    desc:\n";
  _out << "    type:\n";
  _out << "    parameters:\n";
  _out << "    subblocks:\n";
}

void
YAMLFormatter::postscript()
{
  _out << "**END YAML DATA**\n";
}

void
YAMLFormatter::print(const std::string & name, const std::string * /*prev_name*/, std::vector<InputParameters *> & param_ptrs)
{
  std::vector<std::string> elements;
  Parser::tokenize(name, elements);

  // If name is empty - abort or Python will freak
  if (name == "")
    return;
  
  std::string spacing = "";
  for (unsigned int i=0; i<elements.size(); ++i)
    spacing += "  ";

  _out << spacing << "- name: " << name << "\n";
  spacing += "  ";

  std::string class_desc, type_str;
  if (param_ptrs[1] != NULL)
  {
    class_desc = param_ptrs[1]->getClassDescription();
    type_str = param_ptrs[1]->get<std::string>("type");
  }
  
  //will print "" if there is no type or desc, which translates to None in python
  _out << spacing << "desc: !!str " << class_desc << "\n";
  _out << spacing << "type: " << type_str << "\n";
  
  _out << spacing << "parameters:\n";
  std::string subblocks = spacing + "subblocks: \n";
  spacing += "  ";

  for (unsigned int i=0; i<param_ptrs.size() && param_ptrs[i]; ++i)
  {
    for (InputParameters::iterator iter = param_ptrs[i]->begin(); iter != param_ptrs[i]->end(); ++iter) 
    {
      std::string name = iter->first;
      // First make sure we want to see this parameter, also block active and type
      if (param_ptrs[i]->isPrivate(iter->first) || name == "active" || name == "type") 
        continue;

      // Block params may be required and will have a doc string
      std::string required = param_ptrs[i]->isParamRequired(iter->first) ? "Yes" : "No";

      _out << spacing << "- name: " << name << "\n";
      _out << spacing << "  required: " << required << "\n";
      _out << spacing << "  default: !!str ";

      //prints the value, which is the default value when dumping the tree
      //because it hasn't been changed
      iter->second->print(_out);

      _out << "\n" << spacing << "  description: |\n    " << spacing
                << param_ptrs[i]->getDocString(iter->first) << "\n";
    }
  }

  //if there aren't any sub blocks it will just parse as None in python
  _out << subblocks;
}
