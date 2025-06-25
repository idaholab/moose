//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef NEML2_ENABLED

// MOOSE includes
#include "NEML2CentralDifference.h"

registerMooseObject("SolidMechanicsApp", NEML2CentralDifference);

InputParameters
NEML2CentralDifference::validParams()
{
  InputParameters params = ExplicitMixedOrder::validParams();
  params.addClassDescription(
      "Central difference time integrator using NEML2 material models and kernels.");
  params.addRequiredParam<UserObjectName>(
      "assembly", "The NEML2Assembly object to use to provide assembly information");
  params.addRequiredParam<UserObjectName>(
      "fe", "The NEML2FEInterpolation object to use to couple variables");
  return params;
}

NEML2CentralDifference::NEML2CentralDifference(const InputParameters & parameters)
  : ExplicitMixedOrder(parameters)
{
}

void
NEML2CentralDifference::initialSetup()
{
  ExplicitMixedOrder::initialSetup();
  _neml2_assembly = &_fe_problem.getUserObject<NEML2Assembly>("assembly");
  _fe = &_fe_problem.getUserObject<NEML2FEInterpolation>("fe");
}

void
NEML2CentralDifference::postSolve()
{
  ExplicitMixedOrder::postSolve();
  _fe->invalidateInterpolations();
}

void
NEML2CentralDifference::evaluateRHSResidual()
{
  if (_fe->contextUpToDate() && _neml2_assembly->upToDate())
  {
    libMesh::ConstElemRange null_elem_range(&_no_elem);
    _fe_problem.setCurrentAlgebraicElementRange(&null_elem_range);

    libMesh::ConstNodeRange null_node_range(&_no_node);
    _fe_problem.setCurrentAlgebraicNodeRange(&null_node_range);
  }

  ExplicitMixedOrder::evaluateRHSResidual();

  // Reset the algebraic ranges
  _fe_problem.setCurrentAlgebraicElementRange(nullptr);
  _fe_problem.setCurrentAlgebraicNodeRange(nullptr);
}

#endif // NEML2_ENABLED
