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

InputFileFormatter::InputFileFormatter(std::ostream & out, bool dump_mode)
  :SyntaxFormatterInterface(out, dump_mode)
{
}

void
InputFileFormatter::print(const std::string & name, const std::string * prev_name, std::vector<InputParameters *> & param_ptrs)
{
  std::vector<std::string> elements;
  Parser::tokenize(name, elements);
  std::stringstream ss;
  std::set<std::string> seen_it;

  std::string quotes   = "";
  std::string spacing  = "";
  std::string forward  = "";
  std::string backdots = "";
  int         offset   = 30;
  for (unsigned int i=1; i<elements.size(); ++i)
  {
    spacing += "  ";
    forward = ".";
    offset -= 2;
  }

  /* Not double registered (names don't match) */
  if (prev_name == NULL || *prev_name != name || *prev_name == "Executioner" || prev_name->find("InitialCondition") != std::string::npos)
  {
    printCloseAndOpen(name, prev_name);
    if (name == "")
      return;

    int index = name.find_last_of("/");
    if (index == (int)name.npos)
      index = 0;
    std::string block_name = name.substr(index);
    _out << "\n" << spacing << "[" << forward << block_name << "]";
  }
  
  for (unsigned int i=0; i<param_ptrs.size(); ++i)
  {
    if (param_ptrs[i] == NULL) continue;
    for (InputParameters::iterator iter = param_ptrs[i]->begin(); iter != param_ptrs[i]->end(); ++iter)
    {
      if (_seen_it[name].find(iter->first) != _seen_it[name].end())
        continue;
      
      // We only want non-private params unless we are in dump mode
      if ((_dump_mode && param_ptrs[i]->isPrivate(iter->first)) || ((!_dump_mode && !param_ptrs[i]->isParamValid(iter->first))))
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

      // Insert it into our set so we know not to print the same paramter again if this block is registered
      // multiple times
//      _seen_it[name].insert(iter->first);

      _out << "\n" << spacing << "  " << std::left << std::setw(offset) << iter->first << " = ";
      size_t l_offset = 30;
      if (_dump_mode && param_ptrs[i]->isParamRequired(iter->first))
      {
        _out << "(required)";
        l_offset -= 10;
      }
      else if (!_dump_mode || param_ptrs[i]->isParamValid(iter->first))
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
      // Documentation string
      if (_dump_mode)
      {
        std::vector<std::string> elements;
        std::string doc = param_ptrs[i]->getDocString(iter->first);
        if (doc != "")
        {
          Parser::tokenize(doc, elements, 68, " \t");
          _out << std::right << std::setw(l_offset) << "# " << elements[0];
          for (unsigned int i=1; i<elements.size(); ++i)
            _out << " ...\n" << "  " << std::setw(63) << "# " << elements[i];
        }
      }
    }
  }
}

void
InputFileFormatter::printCloseAndOpen(const std::string & name, const std::string * prev_name) const
{
  std::string empty;
  std::vector<std::string> prev_elements, curr_elements;
  
  if (!prev_name)
    prev_name = &empty;
  
  Parser::tokenize(*prev_name, prev_elements);
  Parser::tokenize(name, curr_elements);

  int num_to_close=0;
  int num_to_open=0;
  int same_elements=0;
  bool first_mismatch = false;
  for (unsigned int i=0; i<curr_elements.size(); ++i)
    if (i >= prev_elements.size())
      ++num_to_open;
    else if (prev_elements[i] != curr_elements[i] || first_mismatch)
    {
      ++num_to_open;
      first_mismatch = true;
    }
    else
      ++same_elements;
  
  // Executioner syntax is different - we'll hack it here!
  if ((name == "Executioner" && *prev_name == "Executioner") ||
      (name.find("InitialCondition") != std::string::npos && prev_name->find("InitialCondition") != std::string::npos)
      || name == "Executioner/Adaptivity")
  {
    num_to_open += 1;
    same_elements -= 1;
  }

  num_to_close = prev_elements.size() - same_elements;
  
  // Close off previous blocks if necessary
  for (int i=1; i<=num_to_close; ++i)
  {
    std::string spacing((prev_elements.size()-i)*2, ' ');
    std::string backdot("[]");
    if (spacing.size())
      backdot = "[../]";
    _out << "\n" << spacing << backdot;
  }
  _out << "\n";

  // Open new blocks if necessary
  for (unsigned int i=curr_elements.size()-num_to_open; i<curr_elements.size()-1 && !curr_elements.empty(); ++i)
  {
    std::string spacing(i*2, ' ');
    _out << "\n" << spacing << "[" << (i == 0 ? "" : "./") << curr_elements[i] << "]";
  }
}
