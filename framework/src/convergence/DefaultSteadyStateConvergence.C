//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DefaultSteadyStateConvergence.h"
#include "FEProblemBase.h"
#include "TransientBase.h"
#include "AuxiliarySystem.h"

registerMooseObject("MooseApp", DefaultSteadyStateConvergence);

InputParameters
DefaultSteadyStateConvergence::validParams()
{
  InputParameters params = DefaultConvergenceBase::validParams();
  params += TransientBase::defaultSteadyStateConvergenceParams();

  params.addClassDescription("Default steady-state convergence criteria.");

  return params;
}

DefaultSteadyStateConvergence::DefaultSteadyStateConvergence(const InputParameters & parameters)
  : DefaultConvergenceBase(parameters),

    _steady_state_tolerance(getSharedExecutionerParam<Real>("steady_state_tolerance")),
    _check_aux(getSharedExecutionerParam<bool>("check_aux")),
    _normalize_norm_by_dt(getSharedExecutionerParam<bool>("normalize_solution_diff_norm_by_dt")),

    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _transient_executioner(dynamic_cast<TransientBase * const>(getMooseApp().getExecutioner())),
    _aux_system(_fe_problem.getAuxiliarySystem())
{
  if (!_transient_executioner)
    mooseError("DefaultSteadyStateConvergence can only be used for Executioners derived from "
               "TransientBase.");
}

void
DefaultSteadyStateConvergence::checkIterationType(IterationType it_type) const
{
  DefaultConvergenceBase::checkIterationType(it_type);

  if (it_type != IterationType::STEADY_STATE)
    mooseError("DefaultSteadyStateConvergence can only be used with steady-state detection.");
}

Convergence::MooseConvergenceStatus
DefaultSteadyStateConvergence::checkConvergence(unsigned int /*iter*/)
{
  TIME_SECTION(_perfid_check_convergence);

  Real norm = 0.0;
  if (_check_aux)
  {
    std::vector<Number> aux_var_diff_norms;
    _aux_system.variableWiseRelativeSolutionDifferenceNorm(aux_var_diff_norms);
    for (auto & aux_var_diff_norm : aux_var_diff_norms)
      aux_var_diff_norm /= (_normalize_norm_by_dt ? _fe_problem.dt() : Real(1));

    norm = *std::max_element(aux_var_diff_norms.begin(), aux_var_diff_norms.end());
  }
  else
    norm = _transient_executioner->computeSolutionChangeNorm(false, _normalize_norm_by_dt);

  _console << "Steady-State Relative Differential Norm: " << norm << std::endl;

  if (norm < _steady_state_tolerance)
    return MooseConvergenceStatus::CONVERGED;
  else
    return MooseConvergenceStatus::ITERATING;
}
