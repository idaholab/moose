//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledVectorDiffusion.h"

registerMooseObject("MooseTestApp", CoupledVectorDiffusion);

InputParameters
CoupledVectorDiffusion::validParams()
{
  InputParameters params = VectorKernel::validParams();
  params.addRequiredCoupledVar("v",
                               "A vector variable that will determine the diffusion rate of u.");
  MooseEnum state("current old older", "current");
  params.addParam<MooseEnum>("state",
                             state,
                             "The 'age' of the gradient you want to use for coupled diffusion. The "
                             "options are: current (the default), old, and older.");
  return params;
}

CoupledVectorDiffusion::CoupledVectorDiffusion(const InputParameters & parameters)
  : VectorKernel(parameters),
    _state(getParam<MooseEnum>("state")),
    _grad_v(_state == "current" ? coupledVectorGradient("v")
                                : (_state == "old" ? coupledVectorGradientOld("v")
                                                   : coupledVectorGradientOlder("v"))),
    _v_id(coupled("v"))
{
}

Real
CoupledVectorDiffusion::computeQpResidual()
{
  return -_grad_v[_qp].contract(_grad_test[_i][_qp]);
}

Real
CoupledVectorDiffusion::computeQpJacobian()
{
  return 0;
}

Real
CoupledVectorDiffusion::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _v_id && _state == "current")
    return -_grad_phi[_j][_qp].contract(_grad_test[_i][_qp]);
  else
    return 0;
}
