//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ContactLineSearch.h"

ContactLineSearch::ContactLineSearch(FEProblemBase & fe_problem,
                                     MooseApp & app,
                                     size_t allowed_lambda_cuts,
                                     Real contact_ltol,
                                     bool affect_ltol)
  : LineSearch(fe_problem, app),
    _current_contact_state(std::make_shared<std::set<dof_id_type>>()),
    _user_ksp_rtol_set(false),
    _allowed_lambda_cuts(allowed_lambda_cuts),
    _contact_ltol(contact_ltol),
    _affect_ltol(affect_ltol)
{
}

ContactLineSearch::~ContactLineSearch() {}

void
ContactLineSearch::printContactInfo()
{
  if (!_current_contact_state->empty())
  {
    // _console << "Node ids in contact: ";
    // for (auto & node_id : _current_contact_state)
    //   _console << node_id << " ";
    // _console << "\n";
    _console << _current_contact_state->size() << " nodes in contact\n";
  }
  else
    _console << "No nodes in contact\n";
}
