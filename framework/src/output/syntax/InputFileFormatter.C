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

#include "InputFileFormatter.h"
#include "Parser.h"

#include <sstream>
#include <vector>

InputFileFormatter::InputFileFormatter(bool dump_mode) :
    SyntaxTree(),
    _dump_mode(dump_mode)
{
}

std::string
InputFileFormatter::printBlockOpen(const std::string &name, short depth, const std::string & /*type*/) const
{
  std::string indent(depth*2, ' ');
  std::string opening_string;

  if (depth)
    opening_string = "./";

  return std::string("\n") + indent + "[" + opening_string + name + "]\n";
}

std::string
InputFileFormatter::printBlockClose(const std::string & /*name*/, short depth) const
{
  std::string indent(depth*2, ' ');
  std::string closing_string;

  if (depth)
    closing_string = "../";

  return std::string("") + indent + "[" + closing_string + "]\n";
}

std::string
InputFileFormatter::printParams(const std::string &prefix, InputParameters &params, short depth, const std::string &search_string, bool &found)
{
  std::stringstream oss;

  std::string quotes   = "";
  std::string spacing  = "";
  std::string forward  = "";
  std::string backdots = "";
  int         offset   = 30;
  for (int i=0; i<depth; ++i)
  {
    spacing += "  ";
    forward = ".";
    offset -= 2;
  }

  for (InputParameters::iterator iter = params.begin(); iter != params.end(); ++iter)
  {
    // We only want non-private params and params that we haven't already seen
    if (params.isPrivate(iter->first) || haveSeenIt(prefix, iter->first))
      continue;

    std::string value;
    if (params.isParamValid(iter->first))
    {
      // Print the parameter's value to a stringstream.
      std::ostringstream toss;
      iter->second->print(toss);
      value = Parser::trim(toss.str());
    }

    // See if we match the search string
    if (wildCardMatch(iter->first, search_string) || wildCardMatch(value, search_string))
    {
      // Don't print active if it is the default all, that means it's not in the input file - unless of course we are in dump mode
      if (!_dump_mode && iter->first == "active")
      {
        libMesh::Parameters::Parameter<std::vector<std::string> > * val = dynamic_cast<libMesh::Parameters::Parameter<std::vector<std::string> >*>(iter->second);
        const std::vector<std::string> & active = val->get();
        if (val != NULL && active.size() == 1 && active[0] == "__all__")
          continue;
      }

      // Mark it as "seen"
      seenIt(prefix, iter->first);

      // Don't print type if it is blank
      if (iter->first == "type")
      {
        libMesh::Parameters::Parameter<std::string> * val = dynamic_cast<libMesh::Parameters::Parameter<std::string>*>(iter->second);
        const std::string & active = val->get();
        if (val != NULL && active == "")
          continue;
      }

      found = true;
      oss << spacing << "  " << std::left << std::setw(offset) << iter->first << " = ";
      size_t l_offset = 30;

      if (!_dump_mode || params.isParamValid(iter->first))
      {
        // If the value has spaces, surround it with quotes, otherwise no quotes
        if (value.find(' ') != std::string::npos)
        {
          quotes = "'";
          l_offset -= 2;
        }
        else
          quotes = "";
        oss << quotes << value << quotes;
        l_offset -= value.size();
      }
      else if (_dump_mode && params.isParamRequired(iter->first))
      {
        oss << "(required)";
        l_offset -= 10;
      }

      // Documentation string
      if (_dump_mode)
      {
        std::vector<std::string> elements;
        std::string doc = params.getDocString(iter->first);
        if (Parser::trim(doc) != "")
        {
          Parser::tokenize(doc, elements, 68, " \t");

          for (unsigned int i=0; i<elements.size(); ++i)
            Parser::escape(elements[i]);

          oss << std::right << std::setw(l_offset) << "# " << elements[0];
          for (unsigned int i=1; i<elements.size(); ++i)
            oss << " ...\n" << "  " << std::setw(63) << "# " << elements[i];
        }
      }
      oss << "\n";
    }
  }

  return oss.str();
}
