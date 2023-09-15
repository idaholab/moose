//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

/*
Material Description of Slip Weakening Friction Law 2D
*/

#include "SlipWeakeningFriction2d.h"
#include "InterfaceKernel.h"

registerMooseObject("TensorMechanicsApp", SlipWeakeningFriction2d);

InputParameters
SlipWeakeningFriction2d::validParams()
{
  InputParameters params = CZMComputeLocalTractionTotalBase::validParams();
  params.addClassDescription("linear slip weakening traction separation law.");
  params.addRequiredParam<Real>("T2_o", "background normal traction");
  params.addRequiredParam<Real>("mu_d", "value of dynamic friction parameter");
  params.addRequiredParam<Real>("Dc", "value of characteristic length");
  params.addRequiredParam<Real>("len", "element edge length");
  params.addRequiredCoupledVar("disp_slipweakening_x", "displacement in x dir");
  params.addRequiredCoupledVar("disp_slipweakening_y", "displacement in y dir");
  params.addRequiredCoupledVar("reaction_slipweakening_x", "reaction in x dir");
  params.addRequiredCoupledVar("reaction_slipweakening_y", "reaction in y dir");
  params.addRequiredCoupledVar("mu_s", "static friction coefficient spatial distribution");
  params.addRequiredCoupledVar("ini_shear_sts", "initial shear stress spatial distribution");
  return params;
}

SlipWeakeningFriction2d::SlipWeakeningFriction2d(const InputParameters & parameters)
  : CZMComputeLocalTractionTotalBase(parameters),
    _T2_o(getParam<Real>("T2_o")),
    _mu_d(getParam<Real>("mu_d")),
    _Dc(getParam<Real>("Dc")),
    _len(getParam<Real>("len")),
    _density(getMaterialPropertyByName<Real>(_base_name + "density")),
    _rot(getMaterialPropertyByName<RankTwoTensor>(_base_name + "czm_total_rotation")),
    _disp_slipweakening_x(coupledValue("disp_slipweakening_x")),
    _disp_slipweakening_neighbor_x(coupledNeighborValue("disp_slipweakening_x")),
    _disp_slipweakening_y(coupledValue("disp_slipweakening_y")),
    _disp_slipweakening_neighbor_y(coupledNeighborValue("disp_slipweakening_y")),
    _reaction_slipweakening_x(coupledValue("reaction_slipweakening_x")),
    _reaction_slipweakening_neighbor_x(coupledNeighborValue("reaction_slipweakening_x")),
    _reaction_slipweakening_y(coupledValue("reaction_slipweakening_y")),
    _reaction_slipweakening_neighbor_y(coupledNeighborValue("reaction_slipweakening_y")),
    _disp_slipweakening_x_old(coupledValueOld("disp_slipweakening_x")),
    _disp_slipweakening_neighbor_x_old(coupledNeighborValueOld("disp_slipweakening_x")),
    _disp_slipweakening_y_old(coupledValueOld("disp_slipweakening_y")),
    _disp_slipweakening_neighbor_y_old(coupledNeighborValueOld("disp_slipweakening_y")),
    _mu_s(coupledValue("mu_s")),
    _ini_shear_sts(coupledValue("ini_shear_sts"))
{
}

void
SlipWeakeningFriction2d::computeInterfaceTractionAndDerivatives()
{
  // Global Displacement Jump
  RealVectorValue displacement_jump_global(
      _disp_slipweakening_x[_qp] - _disp_slipweakening_neighbor_x[_qp],
      _disp_slipweakening_y[_qp] - _disp_slipweakening_neighbor_y[_qp]);

  // Global Displacement Jump Old
  RealVectorValue displacement_jump_old_global(
      _disp_slipweakening_x_old[_qp] - _disp_slipweakening_neighbor_x_old[_qp],
      _disp_slipweakening_y_old[_qp] - _disp_slipweakening_neighbor_y_old[_qp]);

  // Global Displacement Jump Rate
  RealVectorValue displacement_jump_rate_global =
      (displacement_jump_global - displacement_jump_old_global) * (1 / _dt);

  // Local Displacement Jump / Displacement Jump Rate
  RealVectorValue displacement_jump = _rot[_qp].transpose() * displacement_jump_global;
  RealVectorValue displacement_jump_rate = _rot[_qp].transpose() * displacement_jump_rate_global;

  // t is along tangential direction, n is along normal direction
  Real displacement_jump_n = displacement_jump(0);
  Real displacement_jump_t = displacement_jump(1);
  Real displacement_jump_rate_n = displacement_jump_rate(0);
  Real displacement_jump_rate_t = displacement_jump_rate(1);

  // Parameter initialization
  Real tau_f = 0;

  // Reaction force in local coordinate
  RealVectorValue R_plus_global(
      -_reaction_slipweakening_x[_qp], -_reaction_slipweakening_y[_qp], 0);
  RealVectorValue R_minus_global(
      -_reaction_slipweakening_neighbor_x[_qp], -_reaction_slipweakening_neighbor_y[_qp], 0);

  RealVectorValue R_plus_local = _rot[_qp].transpose() * R_plus_global;
  RealVectorValue R_minus_local = _rot[_qp].transpose() * R_minus_global;

  // t is along tangential direction, n is along normal direction
  Real R_plus_local_n = R_plus_local(0);
  Real R_plus_local_t = R_plus_local(1);
  Real R_minus_local_n = R_minus_local(0);
  Real R_minus_local_t = R_minus_local(1);

  // Compute node mass
  Real M = _density[_qp] * _len * _len / 2;

  // Compute T1_o, T2_o for current qp
  Real T1_o = _ini_shear_sts[_qp];
  Real T2_o = _T2_o;

  // Compute sticking stress
  Real T1 = (1 / _dt) * M * displacement_jump_rate_t / (2 * _len) +
            (R_plus_local_t - R_minus_local_t) / (2 * _len) + T1_o;

  Real T2 =
      -(1 / _dt) * M * (displacement_jump_rate_n + (1 / _dt) * displacement_jump_n) / (2 * _len) +
      ((R_minus_local_n - R_plus_local_n) / (2 * _len)) - T2_o;

  // Compute fault traction
  if (T2 < 0)
  {
  }
  else
  {
    T2 = 0;
  }

  // Compute friction strength
  if (std::abs(displacement_jump_t) < _Dc)
  {
    tau_f = (_mu_s[_qp] - (_mu_s[_qp] - _mu_d) * std::abs(displacement_jump_t) / _Dc) *
            (-T2); // square for shear component
  }
  else
  {
    tau_f = _mu_d * (-T2);
  }

  // Compute fault traction
  if (std::abs(T1) < tau_f)
  {
  }
  else
  {
    T1 = tau_f * T1 / std::abs(T1);
  }

  // Assign back traction in CZM
  RealVectorValue traction(T2 + T2_o, -T1 + T1_o, 0);
  _interface_traction[_qp] = traction;
  _dinterface_traction_djump[_qp] = 0;
}
