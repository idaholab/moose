//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "LinearSystem.h"
#include "LineSearch.h"
#include "MooseEnum.h"

#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/linear_implicit_system.h"

registerMooseObject("MooseApp", FEProblem);

InputParameters
FEProblem::validParams()
{
  InputParameters params = FEProblemBase::validParams();
  params.addClassDescription("A normal (default) Problem object that contains a single "
                             "NonlinearSystem and a single AuxiliarySystem object.");

  return params;
}

FEProblem::FEProblem(const InputParameters & parameters)
  : FEProblemBase(parameters), _use_nonlinear(getParam<bool>("use_nonlinear"))
{
  if (_num_nl_sys)
  {
    for (const auto i : index_range(_nl_sys_names))
    {
      const auto & sys_name = _nl_sys_names[i];
      auto & nl = _nl[i];
      nl = _use_nonlinear ? (std::make_shared<NonlinearSystem>(*this, sys_name))
                          : (std::make_shared<MooseEigenSystem>(*this, sys_name));
      _nl_sys.push_back(std::dynamic_pointer_cast<NonlinearSystem>(nl));
      _solver_systems[i] = std::dynamic_pointer_cast<SolverSystem>(nl);
    }

    // backwards compatibility for AD for objects that depend on initializing derivatives during
    // construction
    setCurrentNonlinearSystem(0);
  }

  if (_num_linear_sys)
  {
    if (_num_nl_sys)
      // The logic below indexes _solver_systems as if there are no nonlinear systems, so add this
      // error until that is fixed
      mooseError("MOOSE is not currently equipped to handle both nonlinear and linear systems at "
                 "the same time");
    for (const auto i : index_range(_linear_sys_names))
    {
      _linear_systems[i] = std::make_shared<LinearSystem>(*this, _linear_sys_names[i]);
      _solver_systems[_num_nl_sys + i] =
          std::dynamic_pointer_cast<SolverSystem>(_linear_systems[i]);
    }
  }

  if (_solver_systems.size() > 1)
    for (auto & solver_system : _solver_systems)
      solver_system->system().prefix_with_name(true);

  _aux = std::make_shared<AuxiliarySystem>(*this, "aux0");

  newAssemblyArray(_solver_systems);

  if (_num_nl_sys)
    initNullSpaceVectors(parameters, _nl);

  es().parameters.set<FEProblem *>("_fe_problem") = this;

  // Create extra vectors and matrices if any
  createTagVectors();

  // Create extra solution vectors if any
  createTagSolutions();
}

void
FEProblem::init()
{
  for (const auto nl_sys_index : make_range(_num_nl_sys))
  {
    auto & libmesh_sys = _nl_sys[nl_sys_index]->sys();
    if (libmesh_sys.has_static_condensation())
      for (const auto tid : make_range(libMesh::n_threads()))
        _assembly[tid][nl_sys_index]->addStaticCondensation(libmesh_sys.get_static_condensation());
  }
  for (const auto l_sys_index : make_range(_num_linear_sys))
  {
    auto & libmesh_sys = _linear_systems[l_sys_index]->linearImplicitSystem();
    if (libmesh_sys.has_static_condensation())
      for (const auto tid : make_range(libMesh::n_threads()))
        _assembly[tid][_num_nl_sys + l_sys_index]->addStaticCondensation(
            libmesh_sys.get_static_condensation());
  }

  FEProblemBase::init();
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
  if (enum_line_search == Moose::LS_CONTACT || enum_line_search == Moose::LS_PROJECT)
  {
    if (enum_line_search == Moose::LS_CONTACT)
    {
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
    }
    else
    {
      InputParameters ls_params = _factory.getValidParams("PetscProjectSolutionOntoBounds");
      ls_params.set<FEProblem *>("_fe_problem") = this;

      _line_search = _factory.create<LineSearch>(
          "PetscProjectSolutionOntoBounds", "project_solution_onto_bounds_line_search", ls_params);
    }
  }
  else
    mooseError("Requested line search ", line_search.operator std::string(), " is not supported");
}
