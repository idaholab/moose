/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PlasticHeatEnergy.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<PlasticHeatEnergy>()
{
  InputParameters params = validParams<Kernel>();
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
