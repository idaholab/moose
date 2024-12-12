//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrankNicolson.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"

registerMooseObject("MooseApp", CrankNicolson);

InputParameters
CrankNicolson::validParams()
{
  InputParameters params = TimeIntegrator::validParams();
  params.addClassDescription("Crank-Nicolson time integrator.");
  return params;
}

CrankNicolson::CrankNicolson(const InputParameters & parameters)
  : TimeIntegrator(parameters), _residual_old(addVector("residual_old", false, libMesh::GHOSTED))
{
}

void
CrankNicolson::computeTimeDerivatives()
{
  if (!_sys.solutionUDot())
    mooseError("CrankNicolson: Time derivative of solution (`u_dot`) is not stored. Please set "
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

  for (const auto i : index_range(_du_dot_du))
    if (integratesVar(i))
      _du_dot_du[i] = 2. / _dt;
}

void
CrankNicolson::computeADTimeDerivatives(ADReal & ad_u_dot,
                                        const dof_id_type & dof,
                                        ADReal & /*ad_u_dotdot*/) const
{
  computeTimeDerivativeHelper(ad_u_dot, _solution_old(dof));
}

void
CrankNicolson::init()
{
  TimeIntegrator::init();

  if (!_sys.solutionUDot())
    mooseError("CrankNicolson: Time derivative of solution (`u_dot`) is not stored. Please set "
               "uDotRequested() to true in FEProblemBase befor requesting `u_dot`.");

  // time derivative is assumed to be zero on initial
  NumericVector<Number> & u_dot = *_sys.solutionUDot();
  u_dot.zero();
  computeDuDotDu();

  if (_nl)
  {
    // compute residual for the initial time step
    // Note: we can not directly pass _residual_old in computeResidualTag because
    //       the function will call postResidual, which will cause _residual_old
    //       to be added on top of itself prohibited by PETSc.
    //       Objects executed on initial have been executed by FEProblem,
    //       so we can and should directly call NonlinearSystem residual evaluation.
    _fe_problem.setCurrentResidualVectorTags({_nl->nonTimeVectorTag()});
    _nl->computeResidualTag(_nl->RHS(), _nl->nonTimeVectorTag());
    _fe_problem.clearCurrentResidualVectorTags();

    copyVector(_nl->RHS(), *_residual_old);
  }
}

void
CrankNicolson::postResidual(NumericVector<Number> & residual)
{
  // PETSc 3.19 insists on having closed vectors when doing VecAXPY,
  // and that's probably a good idea with earlier versions too, but
  // we don't always get here with _Re_time closed.
  std::vector<unsigned char> inputs_closed = {
      _Re_time->closed(), _Re_non_time->closed(), _residual_old->closed()};

  // We might have done work on one processor but not all processors,
  // so we have to sync our closed() checks.  Congrats to the BISON
  // folks for test coverage that caught that.
  comm().min(inputs_closed);

  if (!inputs_closed[0])
    _Re_time->close();
  if (!inputs_closed[1])
    _Re_non_time->close();
  if (!inputs_closed[2])
    _residual_old->close();

  if (!_var_restriction)
  {
    residual += *_Re_time;
    residual += *_Re_non_time;
    residual += *_residual_old;
  }
  else
  {
    auto residual_sub = residual.get_subvector(_local_indices);
    auto re_time_sub = _Re_time->get_subvector(_local_indices);
    auto re_non_time_sub = _Re_non_time->get_subvector(_local_indices);
    auto residual_old_sub = _residual_old->get_subvector(_local_indices);
    *residual_sub += *re_time_sub;
    *residual_sub += *re_non_time_sub;
    *residual_sub += *residual_old_sub;
    residual.restore_subvector(std::move(residual_sub), _local_indices);
    _Re_time->restore_subvector(std::move(re_time_sub), _local_indices);
    _Re_non_time->restore_subvector(std::move(re_non_time_sub), _local_indices);
    _residual_old->restore_subvector(std::move(residual_old_sub), _local_indices);
  }
}

void
CrankNicolson::postStep()
{
  copyVector(*_Re_non_time, *_residual_old);
}

Real
CrankNicolson::duDotDuCoeff() const
{
  return 2;
}
