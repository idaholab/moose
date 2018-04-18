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
#include "FEProblemBase.h"

ContactLineSearchBase::ContactLineSearchBase(FEProblemBase & fe_problem,
                                             MooseApp & app,
                                             size_t allowed_lambda_cuts,
                                             Real contact_ltol,
                                             bool affect_ltol)
  : LineSearch(fe_problem, app),
    _user_ksp_rtol_set(false),
    _allowed_lambda_cuts(allowed_lambda_cuts),
    _contact_ltol(contact_ltol),
    _affect_ltol(affect_ltol)
{
}

std::shared_ptr<ContactLineSearchBase>
ContactLineSearchBase::build(const InputParameters & parameters, FEProblemBase & fe_problem)
{
  bool affect_ltol = parameters.isParamValid("contact_line_search_ltol");

  enum LSPackage
  {
    Petsc,
    Moose
  };

  LSPackage ls_package = parameters.get<MooseEnum>("line_search_package").getEnum<LSPackage>();
  switch (ls_package)
  {
#ifdef LIBMESH_HAVE_PETSC
    case Petsc:
      return std::make_shared<Moose::PetscSupport::ContactLineSearch>(
          fe_problem,
          fe_problem.getMooseApp(),
          parameters.get<unsigned>("contact_line_search_allowed_lambda_cuts"),
          affect_ltol ? parameters.get<Real>("contact_line_search_ltol")
                      : parameters.get<Real>("l_tol"),
          affect_ltol);
#endif

    case Moose:
      mooseError("A MOOSE line search has not yet been implemented for contact. Please use the "
                 "Petsc implementation, e.g. 'line_search_package = petsc'.");
      break;

    default:
      mooseError("Invalid line search option specified. Please consider using 'line_search_package "
                 "= petsc'");
      break;
  }
}

void
ContactLineSearchBase::printContactInfo(const std::set<dof_id_type> & contact_set)
{
  if (!contact_set.empty())
  {
    // _console << "Node ids in contact: ";
    // for (auto & node_id : contact_set)
    //   _console << node_id << " ";
    // _console << "\n";
    _console << contact_set.size() << " nodes in contact\n";
  }
  else
    _console << "No nodes in contact\n";
}

void
ContactLineSearchBase::insert_set(const std::set<dof_id_type> & mech_set)
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
  zero_its();
}
