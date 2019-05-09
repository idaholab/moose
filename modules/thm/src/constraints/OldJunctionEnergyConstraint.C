#include "OldJunctionEnergyConstraint.h"
#include "Assembly.h"
#include "SinglePhaseFluidProperties.h"
#include "THMApp.h"
#include "Numerics.h"
#include "MooseVariable.h"
#include "MooseVariableScalar.h"
#include "SystemBase.h"

registerMooseObject("THMApp", OldJunctionEnergyConstraint);

template <>
InputParameters
validParams<OldJunctionEnergyConstraint>()
{
  InputParameters params = validParams<NodalConstraint>();
  params.addRequiredParam<std::vector<Real>>("normals", "node normals");
  params.addRequiredParam<std::vector<dof_id_type>>("nodes", "node IDs");
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addRequiredCoupledVar("rhoA", "Density");
  params.addRequiredCoupledVar("rhouA", "Momentum");
  params.addRequiredCoupledVar("rhoEA", "Total energy");
  params.addRequiredCoupledVar("rho", "Density");
  params.addRequiredCoupledVar("vel", "Velocity");
  params.addRequiredCoupledVar("p_junction", "Pressure at the junction");
  params.addRequiredCoupledVar("energy_junction", "Energy at the junction");
  params.addRequiredParam<Real>("initial_rho", "Initial rho");
  params.addRequiredParam<Real>("ref_area", "Reference area of this junction");
  params.addRequiredParam<std::vector<Real>>("K", "Form loss coefficient across the junction");
  params.addRequiredParam<std::vector<Real>>("K_reverse",
                                             "Reverse form loss coefficient across the junction");
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties user object to use.");
  params.addRequiredCoupledVar("total_mfr_in", "Total mass flow rate into the junction");
  params.addRequiredCoupledVar("total_int_energy_rate_in",
                               "Net thermal energy flowing into the junction");
  return params;
}

OldJunctionEnergyConstraint::OldJunctionEnergyConstraint(const InputParameters & parameters)
  : NodalConstraint(parameters),
    _normals(getParam<std::vector<Real>>("normals")),
    _area(coupledValue("A")),
    _rhoA(coupledValue("rhoA")),
    _rhouA(coupledValue("rhouA")),
    _rhoEA(coupledValue("rhoEA")),
    _rho(coupledValue("rho")),
    _vel(coupledValue("vel")),
    _pressure_junction(coupledScalarValue("p_junction")),
    _energy_junction(coupledScalarValue("energy_junction")),

    _total_mfr_in(coupledScalarValue("total_mfr_in")),
    _total_int_energy_rate_in(coupledScalarValue("total_int_energy_rate_in")),

    _k_coeff(getParam<std::vector<Real>>("K")),
    _kr_coeff(getParam<std::vector<Real>>("K_reverse")),
    _ref_area(getParam<Real>("ref_area")),
    _initial_rho(getParam<Real>("initial_rho")),

    _rhoA_var_number(coupled("rhoA")),
    _rhouA_var_number(coupled("rhouA")),
    _pbr_var_number(coupledScalar("p_junction")),
    _ebr_var_number(coupledScalar("energy_junction")),

    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
  _master_node_vector = getParam<std::vector<dof_id_type>>("nodes");
  // just a dummy value that is never used
  _connected_nodes.push_back(*_master_node_vector.begin());
}

Real
OldJunctionEnergyConstraint::pressure(unsigned int i)
{
  Real specific_thermal_energy_junction = 0; // specific thermal(int) energy in this junction
  // FIXME: 2-phase flow
  if (std::fabs(_total_mfr_in[0]) < std::numeric_limits<Real>::min() * 100.0)
    specific_thermal_energy_junction = _rhoEA[i] / _rhoA[i] - 0.5 * _vel[i] * _vel[i];
  else
    specific_thermal_energy_junction = _total_int_energy_rate_in[0] / _total_mfr_in[0];

  Real pressure_junction = _pressure_junction[0];
  Real T_junction = _fp.T_from_v_e(1 / _initial_rho, specific_thermal_energy_junction);
  Real rho_junction = _fp.rho_from_p_T(pressure_junction, T_junction);
  Real ref_vel = _total_mfr_in[0] / rho_junction / _ref_area;

  Real kinetic_energy_junction = 0.5 * rho_junction * ref_vel * ref_vel;
  Real kinetic_energy_pipe = 0.5 * _rho[i] * _vel[i] * _vel[i];

  if (_normals[i] * _vel[i] > 0) // case of net flow into the junction
  {
    Real pressure_loss = _k_coeff[i] * kinetic_energy_pipe;
    return pressure_junction + kinetic_energy_junction + pressure_loss - kinetic_energy_pipe;
  }
  else // case of net flow out the junction
  {
    Real pressure_loss = _kr_coeff[i] * kinetic_energy_pipe;
    return pressure_junction + kinetic_energy_junction - pressure_loss - kinetic_energy_pipe;
  }
}

Real
OldJunctionEnergyConstraint::energy(unsigned int i)
{
  if (_normals[i] * _vel[i] > 0) // case of net flow into the junction
    return _rhoEA[i];
  else
    return _rhoA[i] * (_energy_junction[0] + 0.5 * _vel[i] * _vel[i]);
}

void
OldJunctionEnergyConstraint::computeResidual(NumericVector<Number> & /*residual*/)
{
  auto && dofs = _var.dofIndices();
  DenseVector<Number> re(dofs.size());

  re.zero();
  for (unsigned int i = 0; i < dofs.size(); i++)
    re(i) = (energy(i) + pressure(i) * _area[i]) * _vel[i] * _normals[i];
  re *= _var.scalingFactor();
  _assembly.cacheResidualNodes(re, dofs);
}

Real OldJunctionEnergyConstraint::computeQpResidual(Moose::ConstraintType /*type*/) { return 0; }

void
OldJunctionEnergyConstraint::computeJacobian(SparseMatrix<Number> & /*jacobian*/)
{
  auto && dofs = _var.dofIndices();

  unsigned int n = dofs.size(); // number of connections in the "junction"

  MooseVariable & var_arhoA = _sys.getFieldVariable<Real>(_tid, _rhoA_var_number);
  MooseVariable & var_arhouA = _sys.getFieldVariable<Real>(_tid, _rhouA_var_number);
  MooseVariable & var_arhoEA = _sys.getFieldVariable<Real>(_tid, _var.number());
  MooseVariableScalar & var_pbr = _sys.getScalarVariable(_tid, _pbr_var_number);
  MooseVariableScalar & var_ebr = _sys.getScalarVariable(_tid, _ebr_var_number);

  auto && dofs_arhoA = var_arhoA.dofIndices();
  auto && dofs_arhouA = var_arhouA.dofIndices();
  auto && dofs_arhoEA = var_arhoEA.dofIndices();
  auto && dofs_pbr = var_pbr.dofIndices();
  auto && dofs_ebr = var_ebr.dofIndices();

  DenseMatrix<Number> Kee_arhoA(dofs.size(), dofs_arhoA.size());
  DenseMatrix<Number> Kee_arhouA(dofs.size(), dofs_arhouA.size());
  DenseMatrix<Number> Kee_arhoEA(dofs.size(), dofs_arhoEA.size());
  DenseMatrix<Number> Kee_pbr(dofs.size(), dofs_pbr.size());
  DenseMatrix<Number> Kee_ebr(dofs.size(), dofs_ebr.size());

  Kee_arhoA.zero();
  Kee_arhouA.zero();
  Kee_arhoEA.zero();
  Kee_pbr.zero();
  Kee_ebr.zero();

  for (_i = 0; _i < n; _i++)
  {
    Real specific_thermal_energy_junction = 0; // specific thermal(int) energy in this junction
    // FIXME: 2-phase flow
    if (std::fabs(_total_mfr_in[0]) < std::numeric_limits<Real>::min() * 100.0)
      specific_thermal_energy_junction = _rhoEA[_i] / _rhoA[_i] - 0.5 * _vel[_i] * _vel[_i];
    else
      specific_thermal_energy_junction = _total_int_energy_rate_in[0] / _total_mfr_in[0];

    Real pressure_junction = _pressure_junction[0];

    Real T_junction, dT_dv, dT_de;
    _fp.T_from_v_e(1 / _initial_rho, specific_thermal_energy_junction, T_junction, dT_dv, dT_de);

    Real rho_junction, drho_junction_dpbr, drho_junction_dTbr;
    _fp.rho_from_p_T(
        pressure_junction, T_junction, rho_junction, drho_junction_dpbr, drho_junction_dTbr);
    Real ref_vel = _total_mfr_in[0] / rho_junction / _ref_area;

    Real dE_debr = 0;
    if (_normals[_i] * _vel[_i] > 0) // case of net flow into the junction
      dE_debr = 0;
    else
      dE_debr = _rhoA[_i];

    for (_j = 0; _j < n; _j++)
    {
      Real de_drhoA = 0, de_drhoEA = 0;
      if (std::fabs(_total_mfr_in[0]) < std::numeric_limits<Real>::min() * 100.0)
      {
        de_drhoA = THM::de_darhoA(_rhoA[_i], _rhouA[_i], _rhoEA[_i]);
        de_drhoEA = THM::de_darhoEA(_rhoA[_i]);
      }
      else
      {
        de_drhoA = _normals[_j] * _rhouA[_j] * THM::de_darhoA(_rhoA[_j], _rhouA[_j], _rhoEA[_j]) /
                   _total_mfr_in[0];
        de_drhoEA = _normals[_j] * _rhouA[_j] * THM::de_darhoEA(_rhoA[_j]) / _total_mfr_in[0];
      }
      Real dT_drhoA = dT_de * de_drhoA;
      Real dT_drhoEA = dT_de * de_drhoEA;

      Real drho_junction_darhoA = drho_junction_dTbr * dT_drhoA;
      Real drho_junction_darhoEA = drho_junction_dTbr * dT_drhoEA;

      Real dref_vel_darhoA =
          -_total_mfr_in[0] / rho_junction / rho_junction / _ref_area * drho_junction_darhoA;
      Real dref_vel_darhouA = _normals[_j] / rho_junction / _ref_area;
      Real dref_vel_darhoEA =
          -_total_mfr_in[0] / rho_junction / rho_junction / _ref_area * drho_junction_darhoEA;

      if (_i == _j)
      {
        Real p = pressure(_i);
        Real E = energy(_i);
        Real u2 = _vel[_j] * _vel[_j];
        Real dpA_darhoA = 0;
        Real dpA_darhouA = 0;
        Real dpA_darhoEA = 0;
        Real dE_darhoA = 0;
        Real dE_darhouA = 0;
        Real dE_darhoEA = 0;
        if (_normals[_i] * _vel[_i] > 0) // case of net flow into the junction
        {
          Real dKEbr_darhoA = 0.5 * (drho_junction_darhoA * ref_vel * ref_vel +
                                     rho_junction * 2 * ref_vel * dref_vel_darhoA);
          dpA_darhoA += -0.5 * u2 * (_k_coeff[_i] - 1) + dKEbr_darhoA * _area[_i];
          dE_darhoA = 0;

          Real dref_vel_darhouA = _normals[_i] / rho_junction / _ref_area;
          dpA_darhouA += 0.5 * rho_junction * 2 * ref_vel * dref_vel_darhouA * _area[_i] +
                         _vel[_i] * (_k_coeff[_i] - 1);
          dE_darhouA = 0;

          Real dKEbr_darhoEA = 0.5 * (drho_junction_darhoEA * ref_vel * ref_vel +
                                      rho_junction * 2 * ref_vel * dref_vel_darhoEA);
          dpA_darhoEA = dKEbr_darhoEA * _area[_i];
          dE_darhoEA = 1;
        }
        else
        {
          dpA_darhoA = 0.5 * u2 * (_kr_coeff[_i] + 1);
          dE_darhoA = _energy_junction[0] - 0.5 * _vel[_i] * _vel[_i];

          dpA_darhouA = -_vel[_i] * (_kr_coeff[_i] + 1);
          dE_darhouA = _vel[_i];

          dpA_darhoEA = 0;
          dE_darhoEA = 0;
        }

        Kee_arhoA(_i, _j) =
            _vel[_i] * (dE_darhoA + dpA_darhoA - (E + p * _area[_i]) / _rhoA[_i]) * _normals[_i];
        Kee_arhouA(_i, _j) =
            ((E + p * _area[_i]) / _rhoA[_i] + _vel[_i] * (dE_darhouA + dpA_darhouA)) *
            _normals[_i];
        Kee_arhoEA(_i, _j) = (dE_darhoEA + dpA_darhoEA) * _vel[_i] * _normals[_i];
      }
      else
      {
        if (_normals[_j] * _vel[_j] > 0) // case of net flow into the junction
        {
          Real dKEbr_darhoA = 0.5 * (drho_junction_darhoA * ref_vel * ref_vel +
                                     rho_junction * 2 * ref_vel * dref_vel_darhoA);
          Real dpA_darhoA = dKEbr_darhoA * _area[_i];
          Kee_arhoA(_i, _j) = dpA_darhoA * _vel[_i] * _normals[_i];

          Real dpA_darhouA = 0.5 * rho_junction * 2 * ref_vel * dref_vel_darhouA * _area[_i];
          Kee_arhouA(_i, _j) = dpA_darhouA * _vel[_i] * _normals[_i];

          Real dKEbr_darhoEA = 0.5 * (drho_junction_darhoEA * ref_vel * ref_vel +
                                      rho_junction * 2 * ref_vel * dref_vel_darhoEA);
          Real dpA_darhoEA = dKEbr_darhoEA * _area[_i];
          Kee_arhoEA(_i, _j) = dpA_darhoEA * _vel[_i] * _normals[_i];
        }
      }
    }

    Real dref_vel2_dpbr = -2 * ref_vel * _total_mfr_in[0] / _ref_area / rho_junction /
                          rho_junction * drho_junction_dpbr;
    Real dKEbr_dpbr =
        0.5 * (drho_junction_dpbr * ref_vel * ref_vel + dref_vel2_dpbr * rho_junction);
    Real dpA_dpbr = (1 + dKEbr_dpbr) * _area[_i];
    Kee_pbr(_i, 0) = dpA_dpbr * _vel[_i] * _normals[_i];

    Kee_ebr(_i, 0) = dE_debr * _vel[_i] * _normals[_i];
  }

  _assembly.cacheJacobianBlock(Kee_arhoA, dofs, dofs_arhoA, _var.scalingFactor());
  _assembly.cacheJacobianBlock(Kee_arhouA, dofs, dofs_arhouA, _var.scalingFactor());
  _assembly.cacheJacobianBlock(Kee_arhoEA, dofs, dofs_arhoEA, _var.scalingFactor());
  _assembly.cacheJacobianBlock(Kee_pbr, dofs, dofs_pbr, _var.scalingFactor());
  _assembly.cacheJacobianBlock(Kee_ebr, dofs, dofs_ebr, _var.scalingFactor());
}

Real OldJunctionEnergyConstraint::computeQpJacobian(Moose::ConstraintJacobianType /*type*/)
{
  return 0;
}
