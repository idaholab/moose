//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PlasticHeatEnergy.h"
#include "MooseMesh.h"
#include "MooseVariable.h"

registerMooseObject("TensorMechanicsApp", PlasticHeatEnergy);

InputParameters
PlasticHeatEnergy::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Plastic heat energy density = coeff * stress * plastic_strain_rate");
  params.addRequiredCoupledVar("displacements",
                               "The string of displacements suitable for the problem statement");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<Real>("coeff", 1.0, "Heat energy density = coeff * stress * plastic_strain_rate");
  return params;
}

PlasticHeatEnergy::PlasticHeatEnergy(const InputParameters & parameters)
  : Kernel(parameters),
    _coeff(getParam<Real>("coeff")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _plastic_heat(getMaterialProperty<Real>(_base_name + "plastic_heat")),
    _dplastic_heat_dstrain(
        getMaterialProperty<RankTwoTensor>(_base_name + "dplastic_heat_dstrain")),
    _ndisp(coupledComponents("displacements")),
    _disp_var(_ndisp)
{
  for (unsigned int i = 0; i < _ndisp; ++i)
    _disp_var[i] = coupled("displacements", i);

  // Checking for consistency between mesh size and length of the provided displacements vector
  if (_ndisp != _mesh.dimension())
    mooseError("PlasticHeatEnergy: The number of displacement variables supplied must match the "
               "mesh dimension.");
}

Real
PlasticHeatEnergy::computeQpResidual()
{
  return -_test[_i][_qp] * _coeff * _plastic_heat[_qp];
}

Real
PlasticHeatEnergy::computeQpJacobian()
{
  return computeQpOffDiagJacobian(_var.number());
}

Real
PlasticHeatEnergy::computeQpOffDiagJacobian(unsigned int jvar)
{
  for (unsigned int i = 0; i < _ndisp; ++i)
    if (jvar == _disp_var[i])
      return -_test[_i][_qp] * _coeff * (_dplastic_heat_dstrain[_qp] * _grad_phi[_j][_qp])(i);

  return 0.0;
}
