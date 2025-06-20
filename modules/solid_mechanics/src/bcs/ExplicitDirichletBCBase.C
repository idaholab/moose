//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExplicitMixedOrder.h"
#include "ExplicitDirichletBCBase.h"
#include "MooseError.h"
#include "NonlinearSystemBase.h"
#include "libmesh/numeric_vector.h"
#include <iostream>
#include <ostream>

InputParameters
ExplicitDirichletBCBase::validParams()
{
  InputParameters params = NodalBC::validParams();
  return params;
}

ExplicitDirichletBCBase::ExplicitDirichletBCBase(const InputParameters & parameters)
  : NodalBC(parameters),
    // _mass_diag(initLumpedMass()),
    _mass_lumped(initMassLumped()),
    _damping_lumped(initDampingLumped()),
    _u_old(_var.nodalValueOld()),
    _u_dot_old(_var.nodalValueDotOld()),
    _explicit_integrator(
        dynamic_cast<const ExplicitMixedOrder *>(&_sys.getTimeIntegrator(_var.number())))
{
  // if (!_has_lumped_matrix)
  //   mooseError("Lumped mass matrix is missing. Make sure ExplicitMixedOrder is being used as the
  //   "
  //              "time integrator.");
  if (!_explicit_integrator)
    mooseError("Time integrator for the variable is not of the right type.");
}

Real
ExplicitDirichletBCBase::computeQpResidual()
{
  // Get dof for current var
  const auto dofnum = _variable->nodalDofIndex();
  Real resid = 0;
  // Compute residual to enforce BC based on time order
  switch (_var_time_order)
  {
    case ExplicitMixedOrder::FIRST:
      resid = (computeQpValue() - _u_old) / _dt;
      resid *= -_mass_lumped(dofnum);
      break;

    case ExplicitMixedOrder::SECOND:
      Real avg_dt = (_dt + _dt_old) / 2;
      resid = (computeQpValue() - _u_old) / (avg_dt * _dt) - (_u_dot_old) / avg_dt;
      resid *= -_mass_lumped(dofnum);
      resid += _damping_lumped(dofnum) * _u_dot_old;
      break;
  }
  return resid;
}

void
ExplicitDirichletBCBase::timestepSetup()
{
  // Now is the point that the time integrator has the variable time orders setup
  _var_time_order = _explicit_integrator->findVariableTimeOrder(_var.number());
}

// const NumericVector<Number> &
// ExplicitDirichletBCBase::initLumpedMass()
// {
//   const auto & nl = _fe_problem.getNonlinearSystemBase(_sys.number());
//   if (nl.hasVector("mass_matrix_diag_inverted"))
//     return nl.getVector("mass_matrix_diag_inverted");

//   mooseError("Lumped mass matrix is missing. Make sure ExplicitMixedOrder is being used as
//   the "
//              "time integrator.");
// }

const NumericVector<Number> &
ExplicitDirichletBCBase::initMassLumped()
{
  const auto & nl = _fe_problem.getNonlinearSystemBase(_sys.number());
  if (nl.hasVector("mass_matrix_lumped"))
    return nl.getVector("mass_matrix_lumped");

  mooseError("Lumped mass matrix is missing. Make sure ExplicitMixedOrder is being used as the "
             "time integrator.");
}

const NumericVector<Number> &
ExplicitDirichletBCBase::initDampingLumped()
{
  const auto & nl = _fe_problem.getNonlinearSystemBase(_sys.number());
  // if (nl.hasVector("damping_matrix_lumped"))
  return nl.getVector("damping_matrix_lumped");
}