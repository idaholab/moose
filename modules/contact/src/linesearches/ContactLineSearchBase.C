//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ContactLineSearchBase.h"
#include "PetscSupport.h"
#include "InputParameters.h"
#include "MooseEnum.h"
#include "FEProblem.h"
#include "MooseError.h"

registerMooseObjectAliased("ContactApp", ContactLineSearchBase, "ContactLineSearch");

InputParameters
ContactLineSearchBase::validParams()
{
  InputParameters params = LineSearch::validParams();
  params.addRequiredParam<unsigned>("allowed_lambda_cuts",
                                    "The number of times lambda is allowed to get cut");
  params.addRequiredParam<Real>("contact_ltol",
                                "The linear tolerance to use when the contact set is changing.");
  params.addRequiredParam<bool>("affect_ltol",
                                "Whether to change the linear tolerance from the default value "
                                "when the contact set is changing");
  MooseEnum line_search_package("petsc moose");
  return params;
}

ContactLineSearchBase::ContactLineSearchBase(const InputParameters & parameters)
  : LineSearch(parameters),
    _user_ksp_rtol_set(false),
    _allowed_lambda_cuts(getParam<unsigned>("allowed_lambda_cuts")),
    _contact_ltol(getParam<Real>("contact_ltol")),
    _affect_ltol(getParam<bool>("affect_ltol"))
{
}

void
ContactLineSearchBase::printContactInfo(const std::set<dof_id_type> & contact_set)
{
  if (!contact_set.empty())
    _console << contact_set.size() << " nodes in contact" << std::endl;
  else
    _console << "No nodes in contact" << std::endl;
}

void
ContactLineSearchBase::insertSet(const std::set<dof_id_type> & mech_set)
{
  if (_current_contact_state.empty())
    _current_contact_state = mech_set;
  else
    for (auto & node : mech_set)
      _current_contact_state.insert(node);
}

void
ContactLineSearchBase::reset()
{
  _current_contact_state.clear();
  zeroIts();
}
