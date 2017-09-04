/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeCrackPropagationHeatEnergy.h"

template <>
InputParameters
validParams<ComputeCrackPropagationHeatEnergy>()
{
  InputParameters params = validParams<Material>();
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.addClassDescription("Crack propagation heat energy density = - (dPsi/dc) * (dc/dt) "
                             "is the energy dissipated due to damage increase "
                             "Psi is the free energy of the phase-field fracture model");
  params.addRequiredCoupledVar("c", "Phase field damage variable");
  params.addCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  return params;
}

ComputeCrackPropagationHeatEnergy::ComputeCrackPropagationHeatEnergy(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _c(coupledValue("c")),
    _c_old(coupledValueOld("c")),
    _c_var(coupled("c")),
    _c_name(getVar("c", 0)->name()),
    _G0_pos(getMaterialPropertyByName<Real>(_base_name + "G0_pos")),
    _dG0_pos_dstrain(getMaterialPropertyByName<RankTwoTensor>(_base_name + "dG0_pos_dstrain")),
    _crack_propagation_heat(declareProperty<Real>(_base_name + "crack_propagation_heat")),
    _dcrack_propagation_heat_dstrain(
        declareProperty<RankTwoTensor>(_base_name + "dcrack_propagation_heat_dstrain")),
    _dcrack_propagation_heat_dc(
        declarePropertyDerivative<Real>(_base_name + "dcrack_propagation_heat_dc", _c_name))
{
}

void
ComputeCrackPropagationHeatEnergy::computeQpProperties()
{
  // Calculate time derivative of the phase field damage variable
  Real cdot = (_c[_qp] - _c_old[_qp]) / _dt;

  // Calculate crack propagation heat source: - (dPsi/dc) * (dc/dt)
  // Psi = (1 - c)^2 * G0_pos + G0_neg
  // - (dPsi/dc) * (dc/dt) = 2 * (1 - c) * G0_pos * (dc/dt)
  // C. Miehe, L.M. Schanzel, H. Ulmer, Comput. Methods Appl. Mech. Engrg. 294 (2015) 449 - 485
  // P. Chakraborty, Y. Zhang, M.R. Tonks, Multi-scale modeling of microstructure dependent
  // inter-granular fracture in UO2 using a phase-field based method
  // Idaho National Laboratory technical report
  Real x = 2.0 * (1.0 - _c[_qp]) * cdot;
  _crack_propagation_heat[_qp] = x * _G0_pos[_qp];

  if (_fe_problem.currentlyComputingJacobian())
  {
    _dcrack_propagation_heat_dstrain[_qp] = x * _dG0_pos_dstrain[_qp];
    _dcrack_propagation_heat_dc[_qp] = -2.0 * _G0_pos[_qp] * cdot;
    _dcrack_propagation_heat_dc[_qp] += 2.0 * _G0_pos[_qp] * (1.0 - _c[_qp]) / _dt;
  }
}
