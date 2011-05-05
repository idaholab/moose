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

#include "Action.h"

#include "Parser.h"

template<>
InputParameters validParams<Action>()
{
  InputParameters params;
  std::vector<std::string> blocks(1);
  blocks[0] = "__all__";

  // Add the "active" parameter to all blocks to support selective child visitation (turn blocks on and off without comments)
  params.addParam<std::vector<std::string> >("active", blocks, "If specified only the blocks named will be visited and made active");
  params.addPrivateParam<std::string>("action");
  params.addPrivateParam<Parser *>("parser_handle");
  return params;
}


Action::Action(const std::string & name, InputParameters params) :
    _name(name),
    _pars(params),
    _action(getParam<std::string>("action")),
    _parser_handle(*getParam<Parser *>("parser_handle")),
    _problem(_parser_handle._problem)
{
}

std::string
Action::getShortName() const
{
  return _name.substr(_name.find_last_of('/') != std::string::npos ? _name.find_last_of('/') + 1 : 0);
}

void
Action::addParamsPtrs(std::vector<InputParameters *> & param_ptrs)
{
  param_ptrs.push_back(&_pars);
}

void
Action::printInputFile(const std::string * prev_name, std::ostream & out)
{
  std::vector<std::string> elements;
  Parser::tokenize(_name, elements);
  std::stringstream ss;

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
  if (prev_name == NULL || *prev_name != _name)
  {
    printCloseAndOpen(prev_name, _name, out);
    if (_name == "")
      return;

    int index = _name.find_last_of("/");
    if (index == (int)_name.npos)
      index = 0;
    std::string block_name = _name.substr(index);
    out << "\n" << spacing << "[" << forward << block_name << "]";
  }

  std::vector<InputParameters *> param_ptrs;
  addParamsPtrs(param_ptrs);
  //param_ptrs.push_back(&_pars);
  //param_ptrs.push_back(&_class_params);

  for (unsigned int i=0; i<param_ptrs.size(); ++i)
  {
    for (InputParameters::iterator iter = param_ptrs[i]->begin(); iter != param_ptrs[i]->end(); ++iter)
    {
      // We only want non-private valid params
      if (param_ptrs[i]->isPrivate(iter->first) || !param_ptrs[i]->isParamValid(iter->first))
        continue;

      // Don't print active if it is the default all, that means it's not in the input file
      if (iter->first == "active")
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

      out << "\n" << spacing << "  " << std::left << std::setw(offset) << iter->first << " = ";

      // Print the parameter's value to a stringstream.
      ss.str("");
      iter->second->print(ss);
      // If the value has spaces, surround it with quotes, otherwise no quotes
      std::string value = ss.str();
      if (value.find(' ') != std::string::npos)
        quotes = "'";
      else
        quotes = "";
      out << quotes << value << quotes;
    }
  }
}


void
Action::printCloseAndOpen(const std::string * prev_name, const std::string & curr_name, std::ostream & out) const
{
  std::string empty;
  std::vector<std::string> prev_elements, curr_elements;
  
  if (!prev_name)
    prev_name = &empty;
  
  Parser::tokenize(*prev_name, prev_elements);
  Parser::tokenize(curr_name, curr_elements);

  int num_to_close=0;
  int num_to_open=0;
  int same_elements=0;
  for (unsigned int i=0; i<curr_elements.size(); ++i)
    if (i >= prev_elements.size())
      ++num_to_open;
    else if (prev_elements[i] != curr_elements[i])
      ++num_to_open;
    else
      ++same_elements;

  num_to_close = prev_elements.size() - same_elements;

  // Close off previous blocks if necessary
  for (int i=1; i<=num_to_close; ++i)
  {
    std::string spacing((prev_elements.size()-i)*2, ' ');
    std::string backdot("[]");
    if (spacing.size())
      backdot = "[../]";
    out << "\n" << spacing << backdot;
  }
  out << "\n";

  // Open new blocks if necessary
  for (int i=0; i<num_to_open-1; ++i)
  {
    std::string spacing(i*2, ' ');
    out << "\n" << spacing << "[" << curr_elements[i] << "]";
  }
}
