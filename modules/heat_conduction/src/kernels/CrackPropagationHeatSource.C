/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CrackPropagationHeatSource.h"
#include "MooseMesh.h"
#include "MooseVariable.h"

template <>
InputParameters
validParams<CrackPropagationHeatSource>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Crack propagation heat energy density = - (dPsi/dc) * (dc/dt) "
                             "is the energy dissipated due to damage increase "
                             "Psi is the free energy of the phase-field fracture model");
  params.addRequiredCoupledVar("c", "Phase field damage variable");
  params.addRequiredCoupledVar("displacements",
                               "The string of displacements suitable for the problem statement");
  params.addParam<std::string>("base_name", "Material property base name");
  return params;
}

CrackPropagationHeatSource::CrackPropagationHeatSource(const InputParameters & parameters)
  : DerivativeMaterialInterface<Kernel>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _crack_propagation_heat(getMaterialProperty<Real>(_base_name + "crack_propagation_heat")),
    _dcrack_propagation_heat_dstrain(
        getMaterialProperty<RankTwoTensor>(_base_name + "dcrack_propagation_heat_dstrain")),
    _dcrack_propagation_heat_dc(
        getMaterialPropertyDerivative<Real>(_base_name + "dcrack_propagation_heat_dc", _c_name)),
    _c(coupledValue("c")),
    _c_var(coupled("c")),
    _c_name(getVar("c", 0)->name()),
    _ndisp(coupledComponents("displacements")),
    _disp_var(_ndisp)
{
  for (unsigned int i = 0; i < _ndisp; ++i)
    _disp_var[i] = coupled("displacements", i);

  // Checking for consistency between mesh size and length of the provided displacements vector
  if (_ndisp != _mesh.dimension())
    mooseError(
        "CrackPropagationHeatSource: The number of displacement variables supplied must match the "
        "mesh dimension.");
}

Real
CrackPropagationHeatSource::computeQpResidual()
{
  return -_test[_i][_qp] * _crack_propagation_heat[_qp];
}

Real
CrackPropagationHeatSource::computeQpJacobian()
{
  return computeQpOffDiagJacobian(_var.number());
}

Real
CrackPropagationHeatSource::computeQpOffDiagJacobian(unsigned int jvar)
{
  for (unsigned int i = 0; i < _ndisp; ++i)
    if (jvar == _disp_var[i])
      return -_test[_i][_qp] * (_dcrack_propagation_heat_dstrain[_qp] * _grad_phi[_j][_qp])(i);

  if (jvar == _c_var)
    return -_test[_i][_qp] * _phi[_j][_qp] * _dcrack_propagation_heat_dc[_qp];

  return 0.0;
}
