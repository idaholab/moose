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

InputFileFormatter::InputFileFormatter(std::ostream &out, bool dump_mode) :
    SyntaxTree(),
    _out(out),
    _dump_mode(dump_mode)
{
}

void
InputFileFormatter::printBlockOpen(const std::string &name, short depth, const std::string & /*type*/) const
{
  std::string indent(depth*2, ' ');
  std::string opening_string;

  if (depth)
    opening_string = "./";

  _out << "\n" << indent << "[" << opening_string <<  name << "]";
}

void
InputFileFormatter::printBlockClose(const std::string & /*name*/, short depth) const
{
  std::string indent(depth*2, ' ');
  std::string closing_string;

  if (depth)
    closing_string = "../";

  _out << "\n" << indent << "[" << closing_string << "]\n";
}

void
InputFileFormatter::printParams(InputParameters &params, short depth) const
{
  bool _dump_mode = true;

  std::stringstream ss;
  std::set<std::string> seen_it;

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
    // We only want non-private params unless we are in dump mode
    if ((_dump_mode && params.isPrivate(iter->first)) || ((!_dump_mode && !params.isParamValid(iter->first))))
      continue;

    // Don't print active if it is the default all, that means it's not in the input file - unless of course we are in dump mode
    if (!_dump_mode && iter->first == "active")
    {
      libMesh::Parameters::Parameter<std::vector<std::string> > * val = dynamic_cast<libMesh::Parameters::Parameter<std::vector<std::string> >*>(iter->second);
      const std::vector<std::string> & active = val->get();
      if (val != NULL && active.size() == 1 && active[0] == "__all__")
        continue;
    }

    // Don't print type if it is blank
    if (iter->first == "type")
    {
      libMesh::Parameters::Parameter<std::string> * val = dynamic_cast<libMesh::Parameters::Parameter<std::string>*>(iter->second);
      const std::string & active = val->get();
      if (val != NULL && active == "")
        continue;
    }

    _out << "\n" << spacing << "  " << std::left << std::setw(offset) << iter->first << " = ";
    size_t l_offset = 30;
    if (!_dump_mode || params.isParamValid(iter->first))
    {
      // Print the parameter's value to a stringstream.
      ss.str("");
      iter->second->print(ss);
      // If the value has spaces, surround it with quotes, otherwise no quotes
      std::string value = Parser::trim(ss.str());
      if (value.find(' ') != std::string::npos)
      {
        quotes = "'";
        l_offset -= 2;
      }
      else
        quotes = "";
      _out << quotes << value << quotes;
      l_offset -= value.size();
    }
    else if (_dump_mode && params.isParamRequired(iter->first))
    {
      _out << "(required)";
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

        _out << std::right << std::setw(l_offset) << "# " << elements[0];
        for (unsigned int i=1; i<elements.size(); ++i)
          _out << " ...\n" << "  " << std::setw(63) << "# " << elements[i];
      }
    }
  }
}
