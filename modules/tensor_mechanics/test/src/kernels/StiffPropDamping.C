//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

/*
Kernel of Stiffness Proportional Damping
*/

#include "StiffPropDamping.h"
#include "ElasticityTensorTools.h"

registerMooseObject("TensorMechanicsTestApp", StiffPropDamping);

InputParameters
StiffPropDamping::validParams()
{
  InputParameters params = StressDivergenceTensors::validParams();
  params.addClassDescription("Compute Stiffness Proportional Damping Residual");

  params.addParam<Real>(
      "q", 0.1, "Ratio Factor to assign magnitude of stiffness proportional damping term");

  return params;
}

StiffPropDamping::StiffPropDamping(const InputParameters & parameters)
  : StressDivergenceTensors(parameters),

    // Get stress tensor from previous time step
    _stress_older(getMaterialPropertyOlderByName<RankTwoTensor>(_base_name + "stress")),

    // Get stress tensor from current time step
    _stress(getMaterialPropertyByName<RankTwoTensor>(_base_name + "stress")),

    // Ratio factor
    _q(getParam<Real>("q"))

{
}

Real
StiffPropDamping::computeQpResidual()
{

  Real residual = 0.0;

  residual += _q * (_stress[_qp].row(_component) - _stress_older[_qp].row(_component)) *
              _grad_test[_i][_qp];

  return residual;
}
