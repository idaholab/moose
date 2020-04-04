//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AllenCahnElasticEnergyOffDiag.h"
#include "MathUtils.h"
#include "RankTwoTensor.h"

registerMooseObject("PhaseFieldApp", AllenCahnElasticEnergyOffDiag);

InputParameters
AllenCahnElasticEnergyOffDiag::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("This kernel calculates off-diagonal Jacobian of elastic energy in "
                             "AllenCahn with respect to displacements");
  params.addCoupledVar("displacements",
                       "The vector of displacements suitable for the problem statement");
  params.addParam<MaterialPropertyName>(
      "F_name", "E_el", "Name of material property storing the elastic energy");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  return params;
}

AllenCahnElasticEnergyOffDiag::AllenCahnElasticEnergyOffDiag(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>(parameters),
    _L(getMaterialProperty<Real>("mob_name")),
    _disp_map(getParameterJvarMap("displacements")),
    _d2Fdcdstrain(getMaterialProperty<RankTwoTensor>("d2Fdcdstrain"))
{
}

Real
AllenCahnElasticEnergyOffDiag::computeQpOffDiagJacobian(unsigned int jvar)
{
  auto dispvar = mapJvarToCvar(jvar, _disp_map);
  if (dispvar >= 0)
  {
    const Real dxddFdc = _L[_qp] * _test[_i][_qp];
    const Real d2Fdcdstrain_comp =
        (_d2Fdcdstrain[_qp].column(dispvar) + _d2Fdcdstrain[_qp].row(dispvar)) / 2.0 *
        _grad_phi[_j][_qp];
    return dxddFdc * d2Fdcdstrain_comp;
  }

  return 0.0;
}
