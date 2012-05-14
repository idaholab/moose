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

YAMLFormatter::YAMLFormatter(std::ostream &out, bool dump_mode)
  :SyntaxTree(),
   _out(out),
   _dump_mode(dump_mode),
   _first(true)
{
}

void
YAMLFormatter::preamble()
{
  //important: start and end yaml data delimiters used by python
  _out << "**START YAML DATA**\n";
  _first = true;
}

void
YAMLFormatter::postscript()
{
  _out << "**END YAML DATA**\n";
}

void
YAMLFormatter::printParams(InputParameters &params, short depth) const
{
  std::string indent(depth*2, ' ');

  for (InputParameters::iterator iter = params.begin(); iter != params.end(); ++iter)
  {
    std::string name = iter->first;
    // First make sure we want to see this parameter, also block active and type
    if (params.isPrivate(iter->first) || name == "active")
      continue;

    // Block params may be required and will have a doc string
    std::string required = params.isParamRequired(iter->first) ? "Yes" : "No";

    _out << indent << "  - name: " << name << "\n";
    _out << indent << "    required: " << required << "\n";
    _out << indent << "    default: !!str ";

    //prints the value, which is the default value when dumping the tree
    //because it hasn't been changed

    // remove additional '\n' possibly generated in output (breaks YAML parsing)
    std::ostringstream oss;
    iter->second->print(oss);
    std::string tmp_str = oss.str();
    for(std::string::iterator it=tmp_str.begin(); it!=tmp_str.end(); ++it)
    {
      if ( *it == '\n')
      {
        *it = ' ';
      }
    }

    std::string doc = params.getDocString(iter->first);
    Parser::escape(doc);
    _out << tmp_str;
    _out << "\n" << indent << "    description: |\n      " << indent
         << doc << "\n";
  }
}

void
YAMLFormatter::preTraverse(short depth) const
{
  std::string indent(depth*2, ' ');

  _out << indent << "  subblocks:\n";
}


void
YAMLFormatter::printBlockOpen(const std::string &name, short depth, const std::string &type) const
{
  std::string indent(depth*2, ' ');

  _out << indent << "- name: " << (name == "*" ? type : name) << "\n";
  _out << indent << "  description: !!str\n";
  _out << indent << "  type: " << type << "\n";
  _out << indent << "  parameters:\n";
}

void
YAMLFormatter::printBlockClose(const std::string &/*name*/, short /*depth*/) const
{
}

//
//void
//YAMLFormatter::printCloseAndOpen(const std::string & name, const std::string * prev_name) const
//{
//  std::string empty;
//  std::vector<std::string> prev_elements, curr_elements;
//
//  if (!prev_name)
//    prev_name = &empty;
//
//  Parser::tokenize(*prev_name, prev_elements);
//  Parser::tokenize(name, curr_elements);
//
//  int num_to_close=0;
//  int num_to_open=0;
//  int same_elements=0;
//  bool first_mismatch = false;
//  for (unsigned int i=0; i<curr_elements.size(); ++i)
//    if (i >= prev_elements.size())
//      ++num_to_open;
//    else if (prev_elements[i] != curr_elements[i] || first_mismatch)
//    {
//      ++num_to_open;
//      first_mismatch = true;
//    }
//    else
//      ++same_elements;
//
//  // Executioner syntax is different - we'll hack it here!
//
//  if (/*(name == "Executioner" && *prev_name == "Executioner") ||*/
//    (name.find("InitialCondition") != std::string::npos && prev_name->find("InitialCondition") != std::string::npos)/* || name == "Executioner/Adaptivity"*/)
//  {
//    num_to_open += 1;
//    same_elements -= 1;
//  }
//
//  num_to_close = prev_elements.size() - same_elements;
//
//  // Open new blocks if necessary
//  std::string spacing = "";
//  std::string partial_name = "";
//  bool ran_once = false;
//  for (unsigned int i=0; i<curr_elements.size()-num_to_open; ++i)
//  {
//    spacing += "  ";
//    if (i)
//    {
//
//      partial_name += "/";
//    }
//    partial_name += curr_elements[i];
//  }
//
//  for (unsigned int i=curr_elements.size()-num_to_open; i<curr_elements.size()-1 && !curr_elements.empty(); ++i)
//  {
//    spacing += "  ";
//    if (i)
//    {
//
//      partial_name += "/";
//    }
//    partial_name += curr_elements[i];
//
//    ran_once = true;
//    _out << spacing << "- name: " << partial_name << "\n";
//    _out << spacing << "  desc: !!str\n";
//    _out << spacing << "  type:\n";
//    _out << spacing << "  parameters:\n";
//    _out << spacing << "  subblocks:\n";
//  }
//}
