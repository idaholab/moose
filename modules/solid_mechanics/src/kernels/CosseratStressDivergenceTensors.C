//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CosseratStressDivergenceTensors.h"
#include "Material.h"
#include "RankFourTensor.h"
#include "ElasticityTensorTools.h"
#include "RankTwoTensor.h"
#include "MooseMesh.h"

registerMooseObject("TensorMechanicsApp", CosseratStressDivergenceTensors);

InputParameters
CosseratStressDivergenceTensors::validParams()
{
  InputParameters params = StressDivergenceTensors::validParams();
  params.addRequiredCoupledVar("Cosserat_rotations", "The 3 Cosserat rotation variables");
  return params;
}

CosseratStressDivergenceTensors::CosseratStressDivergenceTensors(const InputParameters & parameters)
  : StressDivergenceTensors(parameters),
    _nrots(coupledComponents("Cosserat_rotations")),
    _wc_var(_nrots)
{
  for (unsigned i = 0; i < _nrots; ++i)
    _wc_var[i] = coupled("Cosserat_rotations", i);
}

Real
CosseratStressDivergenceTensors::computeQpOffDiagJacobian(unsigned int jvar)
{
  for (unsigned int v = 0; v < _nrots; ++v)
    if (jvar == _wc_var[v])
      return ElasticityTensorTools::elasticJacobianWC(
          _Jacobian_mult[_qp], _component, v, _grad_test[_i][_qp], _phi[_j][_qp]);

  return StressDivergenceTensors::computeQpOffDiagJacobian(jvar);
}
