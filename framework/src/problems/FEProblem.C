//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FEProblem.h"

#include "Assembly.h"
#include "AuxiliarySystem.h"
#include "MooseEigenSystem.h"
#include "NonlinearSystem.h"
#include "LineSearch.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/petsc_matrix.h"

registerMooseObject("MooseApp", FEProblem);

template <>
InputParameters
validParams<FEProblem>()
{
  InputParameters params = validParams<FEProblemBase>();
  return params;
}

FEProblem::FEProblem(const InputParameters & parameters)
  : FEProblemBase(parameters),
    _use_nonlinear(getParam<bool>("use_nonlinear")),
    _nl_sys(_use_nonlinear ? (std::make_shared<NonlinearSystem>(*this, "nl0"))
                           : (std::make_shared<MooseEigenSystem>(*this, "eigen0")))
{
  _nl = _nl_sys;
  _aux = std::make_shared<AuxiliarySystem>(*this, "aux0");

  newAssemblyArray(*_nl_sys);

  initNullSpaceVectors(parameters, *_nl_sys);

  _eq.parameters.set<FEProblem *>("_fe_problem") = this;

  // Create extra vectors and matrices if any
  createTagVectors();
}

void
FEProblem::setInputParametersFEProblem(InputParameters & parameters)
{
  // set _fe_problem
  FEProblemBase::setInputParametersFEProblem(parameters);
  // set _fe_problem
  parameters.set<FEProblem *>("_fe_problem") = this;
}

void
FEProblem::addLineSearch(const InputParameters & parameters)
{
  MooseEnum line_search = parameters.get<MooseEnum>("line_search");
  Moose::LineSearchType enum_line_search = Moose::stringToEnum<Moose::LineSearchType>(line_search);
  if (enum_line_search == Moose::LS_CONTACT)
  {
#ifdef LIBMESH_HAVE_PETSC
#if PETSC_VERSION_LESS_THAN(3, 6, 0)
    mooseError("Shell line searches only became available in Petsc in version 3.6.0!");
#else
    InputParameters ls_params = _factory.getValidParams("PetscContactLineSearch");

    bool affect_ltol = parameters.isParamValid("contact_line_search_ltol");
    ls_params.set<bool>("affect_ltol") = affect_ltol;
    ls_params.set<unsigned>("allowed_lambda_cuts") =
        parameters.get<unsigned>("contact_line_search_allowed_lambda_cuts");
    ls_params.set<Real>("contact_ltol") = affect_ltol
                                              ? parameters.get<Real>("contact_line_search_ltol")
                                              : parameters.get<Real>("l_tol");
    ls_params.set<FEProblem *>("_fe_problem") = this;

    _line_search =
        _factory.create<LineSearch>("PetscContactLineSearch", "contact_line_search", ls_params);
#endif
#else
    mooseError("Currently contact line search requires use of Petsc.");
#endif
  }
}

void
FEProblem::init()
{
#ifdef LIBMESH_HAVE_PETSC
  Moose::PetscSupport::petscSetOptions(*this); // Make sure the PETSc options are setup for this app
                                               // Determine if HYPRE is used
#if !PETSC_VERSION_LESS_THAN(3, 9, 2)
  if (_hypre_matrix)
  {
    bool use_hypre = Moose::PetscSupport::detectHypreFromOptions();
    if (use_hypre)
    {
      PetscMatrix<Number> * petsc_mat = dynamic_cast<PetscMatrix<Number> *>(_nl_sys->sys().matrix);
      if (petsc_mat)
        petsc_mat->set_matrix_type(libMesh::HYPRE);
    }
  }
#endif

#endif
  FEProblemBase::init();
}
