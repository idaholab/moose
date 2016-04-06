/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CosseratStressDivergenceTensors.h"
#include "Material.h"
#include "RankFourTensor.h"
#include "RankTwoTensor.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<CosseratStressDivergenceTensors>()
{
  InputParameters params = validParams<StressDivergenceTensors>();
  params.addCoupledVar("wc_x", "The Cosserat rotation about x");
  params.addCoupledVar("wc_y", "The Cosserat rotation about y");
  params.addCoupledVar("wc_z", "The Cosserat rotation about z");
  return params;
}

CosseratStressDivergenceTensors::CosseratStressDivergenceTensors(const InputParameters & parameters) :
    StressDivergenceTensors(parameters),
    _wc_x_var(coupled("wc_x")),
    _wc_y_var(coupled("wc_y")),
    _wc_z_var(coupled("wc_z"))
{
}

Real
CosseratStressDivergenceTensors::computeQpOffDiagJacobian(unsigned int jvar)
{
  unsigned int coupled_component = 3;

  // What does 2D look like here?
  if (jvar == _wc_x_var)
    coupled_component = 0;
  else if (jvar == _wc_y_var)
    coupled_component = 1;
  else if (jvar == _wc_z_var)
    coupled_component = 2;

  if (coupled_component < 3)
    return _Jacobian_mult[_qp].elasticJacobianwc(_component, coupled_component, _grad_test[_i][_qp], _phi[_j][_qp]);

  return StressDivergenceTensors::computeQpOffDiagJacobian(jvar);
}

