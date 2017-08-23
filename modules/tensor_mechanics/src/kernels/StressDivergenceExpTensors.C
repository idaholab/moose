/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StressDivergenceExpTensors.h"
#include "ElasticityTensorTools.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SystemBase.h"

template <>
InputParameters
validParams<StressDivergenceExpTensors>()
{
  InputParameters params = validParams<StressDivergenceTensors>();
  params.addClassDescription("Stress divergence kernel for phase-field fracture: Additionally "
                             "computes off diagonal damage dependent Jacobian components");

  return params;
}

StressDivergenceExpTensors::StressDivergenceExpTensors(const InputParameters & parameters)
  : DerivativeMaterialInterface<StressDivergenceTensors>(parameters),
    _stress_old(getMaterialPropertyOldByName<RankTwoTensor>(_base_name + "stress"))
{
}

Real
StressDivergenceExpTensors::computeQpResidual()
{
  Real residual = _stress_old[_qp].row(_component) * _grad_test[_i][_qp];
  // volumetric locking correction
  if (_volumetric_locking_correction)
    residual += _stress_old[_qp].trace() / 3.0 *
                (_avg_grad_test[_i][_component] - _grad_test[_i][_qp](_component));

  return residual;
}

Real
StressDivergenceExpTensors::computeQpJacobian()
{
  return 0.0;
}

Real
StressDivergenceExpTensors::computeQpOffDiagJacobian(unsigned int jvar)
{
  return 0.0;
}
