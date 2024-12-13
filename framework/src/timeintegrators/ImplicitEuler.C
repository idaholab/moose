//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ImplicitEuler.h"
#include "NonlinearSystem.h"

registerMooseObject("MooseApp", ImplicitEuler);

InputParameters
ImplicitEuler::validParams()
{
  InputParameters params = TimeIntegrator::validParams();
  params.addClassDescription("Time integration using the implicit Euler method.");
  return params;
}

ImplicitEuler::ImplicitEuler(const InputParameters & parameters) : TimeIntegrator(parameters) {}

ImplicitEuler::~ImplicitEuler() {}

void
ImplicitEuler::computeTimeDerivatives()
{
  if (!_sys.solutionUDot())
    mooseError("ImplicitEuler: Time derivative of solution (`u_dot`) is not stored. Please set "
               "uDotRequested() to true in FEProblemBase befor requesting `u_dot`.");

  NumericVector<Number> & u_dot = *_sys.solutionUDot();
  if (!_var_restriction)
  {
    u_dot = *_solution;
    computeTimeDerivativeHelper(u_dot, _solution_old);
  }
  else
  {
    auto u_dot_sub = u_dot.get_subvector(_local_indices);
    _solution->create_subvector(*_solution_sub, _local_indices, false);
    _solution_old.create_subvector(*_solution_old_sub, _local_indices, false);
    *u_dot_sub = *_solution_sub;
    computeTimeDerivativeHelper(*u_dot_sub, *_solution_old_sub);
    u_dot.restore_subvector(std::move(u_dot_sub), _local_indices);
    // Scatter info needed for ghosts
    u_dot.close();
  }

  computeDuDotDu();
}

void
ImplicitEuler::computeADTimeDerivatives(ADReal & ad_u_dot,
                                        const dof_id_type & dof,
                                        ADReal & /*ad_u_dotdot*/) const
{
  computeTimeDerivativeHelper(ad_u_dot, _solution_old(dof));
}

void
ImplicitEuler::postResidual(NumericVector<Number> & residual)
{
  if (!_var_restriction)
  {
    residual += *_Re_time;
    residual += *_Re_non_time;
    residual.close();
  }
  else
  {
    auto residual_sub = residual.get_subvector(_local_indices);
    auto re_time_sub = _Re_time->get_subvector(_local_indices);
    auto re_non_time_sub = _Re_non_time->get_subvector(_local_indices);
    *residual_sub += *re_time_sub;
    *residual_sub += *re_non_time_sub;
    residual.restore_subvector(std::move(residual_sub), _local_indices);
    _Re_time->restore_subvector(std::move(re_time_sub), _local_indices);
    _Re_non_time->restore_subvector(std::move(re_non_time_sub), _local_indices);
  }
}

Real
ImplicitEuler::timeDerivativeRHSContribution(dof_id_type dof_id,
                                             const std::vector<Real> & factors) const
{
  mooseAssert(factors.size() == numStatesRequired(),
              "Either too many or too few states are given!");
  return factors[0] * _solution_old(dof_id) / _dt;
}

Real
ImplicitEuler::timeDerivativeMatrixContribution(const Real factor) const
{
  return factor / _dt;
}
