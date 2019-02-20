#include "VolumeJunctionOldBC.h"
#include "SinglePhaseFluidProperties.h"
#include "Enums.h"

registerMooseObject("THMApp", VolumeJunctionOldBC);

template <>
InputParameters
validParams<VolumeJunctionOldBC>()
{
  InputParameters params = validParams<OneDIntegratedBC>();
  params.addRequiredParam<MooseEnum>("eqn_name",
                                     FlowModel::getFlowEquationType(),
                                     "The name of the equation this BC is acting on");
  // Coupled variables
  params.addRequiredCoupledVar("A", "Area");
  params.addRequiredCoupledVar("rhoA", "Density");
  params.addRequiredCoupledVar("rhouA", "Momentum");
  params.addRequiredCoupledVar("rhoEA", "Total energy");
  params.addRequiredCoupledVar("rho", "Density");
  params.addRequiredCoupledVar("vel", "x velocity");
  params.addRequiredCoupledVar("rho_junction", "Density at the junction");
  params.addRequiredCoupledVar("rhoe_junction", "Energy at the junction");
  params.addRequiredCoupledVar("vel_junction", "Velocity at the junction");
  params.addRequiredCoupledVar("p_junction", "Pressure variable for the junction");
  params.addRequiredCoupledVar("total_mfr_in", "Total mass flow rate going into the junction");
  params.addRequiredParam<Real>("K", "Form loss coefficients");
  params.addRequiredParam<Real>("K_reverse", "Reverse form loss coefficients");
  params.addRequiredParam<Real>("ref_area", "Reference area of this junction");
  params.addRequiredParam<Real>("deltaH",
                                "Height difference between pipe BC and VolumeJunctionOld center");

  params.addRequiredParam<Real>("gravity_magnitude", "Gravitational acceleration magnitude");

  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties user object to use.");

  params.declareControllable("ref_area");

  return params;
}

VolumeJunctionOldBC::VolumeJunctionOldBC(const InputParameters & parameters)
  : OneDIntegratedBC(parameters),
    _eqn_type(THM::stringToEnum<FlowModel::EEquationType>(getParam<MooseEnum>("eqn_name"))),
    _area(coupledValue("A")),
    _rhoA_var_number(coupled("rhoA")),
    _rhoA(coupledValue("rhoA")),
    _rho(coupledValue("rho")),
    _rhouA_var_number(coupled("rhouA")),
    _rhoEA(coupledValue("rhoEA")),
    _vel(coupledValue("vel")),

    _rho_junction_var_number(coupledScalar("rho_junction")),
    _rho_junction(coupledScalarValue("rho_junction")),
    _rhoe_junction_var_number(coupledScalar("rhoe_junction")),
    _rhoe_junction(coupledScalarValue("rhoe_junction")),
    _vel_junction_var_number(coupledScalar("vel_junction")),
    _vel_junction(coupledScalarValue("vel_junction")),
    _pressure_junction(coupledScalarValue("p_junction")),
    _total_mfr_in(coupledScalarValue("total_mfr_in")),

    _k_coeff(getParam<Real>("K")),
    _kr_coeff(getParam<Real>("K_reverse")),
    _ref_area(getParam<Real>("ref_area")),
    _deltaH(getParam<Real>("deltaH")),

    _gravity_magnitude(getParam<Real>("gravity_magnitude")),

    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

Real
VolumeJunctionOldBC::computeQpResidual()
{
  Real rho_junction = _rho_junction[0];
  Real pressure_junction = _pressure_junction[0];
  Real ref_vel = _total_mfr_in[0] / rho_junction / _ref_area;
  Real kinetic_energy_junction = 0.5 * rho_junction * ref_vel * ref_vel;
  Real kinetic_energy_pipe = 0.5 * _rho[_qp] * _vel[_qp] * _vel[_qp];
  Real pressure_acc_loss = kinetic_energy_junction - kinetic_energy_pipe;
  Real pressure_grav_loss = rho_junction * _gravity_magnitude * _deltaH;
  Real pressure_pipe_bc;
  Real rhoEA_pipe_bc;
  if (_normal * _vel[_qp] > 0) // case of net flow into the junction
  {
    Real pressure_form_loss = _k_coeff * kinetic_energy_pipe;
    pressure_pipe_bc =
        pressure_junction + pressure_form_loss + pressure_acc_loss + pressure_grav_loss;
  }
  else // case of net flow out the junction
  {
    Real pressure_form_loss = _kr_coeff * kinetic_energy_pipe;
    pressure_pipe_bc =
        pressure_junction - pressure_form_loss + pressure_acc_loss + pressure_grav_loss;
  }

  if (_eqn_type == FlowModel::MOMENTUM)
  {
    return (pressure_pipe_bc * _area[_qp] + _u[_qp] * _vel[_qp]) * _normal * _test[_i][_qp];
  }
  else if (_eqn_type == FlowModel::ENERGY)
  {
    if (_normal * _vel[_qp] > 0) // case of net flow into the junction
      rhoEA_pipe_bc = _rhoEA[_qp];
    else
      rhoEA_pipe_bc =
          _rhoA[_qp] * (_rhoe_junction[0] / _rho_junction[0] + 0.5 * _vel[_qp] * _vel[_qp]);

    return (rhoEA_pipe_bc + pressure_pipe_bc * _area[_qp]) * _vel[_qp] * _normal * _test[_i][_qp];
  }
  else
    return 0.;
}

Real
VolumeJunctionOldBC::computeQpJacobian()
{
  if (_eqn_type == FlowModel::MOMENTUM)
  {
    Real rho_junction = _rho_junction[0];
    Real ref_vel = _total_mfr_in[0] / rho_junction / _ref_area;

    Real dref_vel_darhouA = 0;
    if (_vel[_qp] * _normal > 0)
      dref_vel_darhouA = _normal / _rho_junction[0] / _ref_area;
    else
      dref_vel_darhouA = 0;

    Real dp_bc_darhouA = 0;
    if (_vel[_qp] * _normal > 0)
      dp_bc_darhouA =
          _vel[_qp] / _area[_qp] * (_k_coeff - 1) + _rho_junction[0] * ref_vel * dref_vel_darhouA;
    else
      dp_bc_darhouA =
          _vel[_qp] / _area[_qp] * (-_kr_coeff - 1) + _rho_junction[0] * ref_vel * dref_vel_darhouA;

    return (dp_bc_darhouA * _area[_qp] + 2 * _vel[_qp]) * _phi[_j][_qp] * _test[_i][_qp] * _normal;
  }
  else if (_eqn_type == FlowModel::ENERGY)
  {
    if (_normal * _vel[_qp] > 0) // case of net flow into the junction
      return _phi[_j][_qp] * _test[_i][_qp] * _vel[_qp] * _normal;
    else
      return 0.0;
  }
  else
    return 0;
}

Real
VolumeJunctionOldBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_eqn_type == FlowModel::MOMENTUM)
  {
    Real rho_junction = _rho_junction[0];
    Real ref_vel = _total_mfr_in[0] / rho_junction / _ref_area;

    if (jvar == _rhoA_var_number)
    {
      Real u2 = _vel[_qp] * _vel[_qp];
      // dF/darhoA = (dp_drho - u^2) * phi(j) * phi(i) * nx
      Real dp_bc_darhoA = 0.5 * u2 / _area[_qp];
      if (_normal * _vel[_qp] > 0) // case of net flow into the junction
        dp_bc_darhoA *= (1 - _k_coeff);
      else
        dp_bc_darhoA *= (1 + _kr_coeff);
      return (dp_bc_darhoA * _area[_qp] - u2) * _phi[_j][_qp] * _test[_i][_qp] * _normal;
    }
    else if (jvar == _rho_junction_var_number)
    {
      Real p, dp_dv, dp_de;
      _fp.p_from_v_e(1. / _rho_junction[0], _rhoe_junction[0] / _rho_junction[0], p, dp_dv, dp_de);
      Real dv_drho_br = -1 / _rho_junction[0] / _rho_junction[0];
      Real de_drho_br = -_rhoe_junction[0] / _rho_junction[0] / _rho_junction[0];
      Real dp_br_drho_br = dp_dv * dv_drho_br + dp_de * de_drho_br;
      Real dref_vel_drho_br = -_total_mfr_in[0] / _ref_area / _rho_junction[0] / _rho_junction[0];
      Real dp_bc_drho_br =
          dp_br_drho_br +
          0.5 * (ref_vel * ref_vel + 2 * rho_junction * ref_vel * dref_vel_drho_br) +
          _gravity_magnitude * _deltaH;
      return dp_bc_drho_br * _area[_qp] * _normal * _test[_i][_qp];
    }
    else if (jvar == _rhoe_junction_var_number)
    {
      Real p, dp_dv, dp_de;
      _fp.p_from_v_e(1. / _rho_junction[0], _rhoe_junction[0] / _rho_junction[0], p, dp_dv, dp_de);
      Real de_drhoe_br = 1 / _rho_junction[0];
      Real dp_br_drhoe_br = dp_de * de_drhoe_br;
      return dp_br_drhoe_br * _area[_qp] * _normal * _test[_i][_qp];
    }
    else if (jvar == _vel_junction_var_number)
    {
      Real dpbr_dvel_br = 0;
      return dpbr_dvel_br * _area[_qp] * _normal * _test[_i][_qp];
    }
  }
  else if (_eqn_type == FlowModel::ENERGY)
  {
    if (jvar == _rhoA_var_number)
    {
      Real rho_junction = _rho_junction[0];
      Real pressure_junction = _pressure_junction[0];
      Real ref_vel = _total_mfr_in[0] / rho_junction / _ref_area;
      Real kinetic_energy_junction = 0.5 * rho_junction * ref_vel * ref_vel;
      Real kinetic_energy_pipe = 0.5 * _rho[_qp] * _vel[_qp] * _vel[_qp];
      Real pressure_acc_loss = kinetic_energy_junction - kinetic_energy_pipe;
      Real pressure_grav_loss = rho_junction * _gravity_magnitude * _deltaH;
      Real pressure_pipe_bc;
      Real rhoEA_bc = 0;
      Real drhoEA_bc_darhoA;
      Real dp_bc_darhoA;
      Real u2 = _vel[_qp] * _vel[_qp];

      if (_normal * _vel[_qp] > 0) // case of net flow into the junction
      {
        rhoEA_bc = _rhoEA[_qp];
        Real pressure_form_loss = _k_coeff * kinetic_energy_pipe;
        pressure_pipe_bc =
            pressure_junction + pressure_form_loss + pressure_acc_loss + pressure_grav_loss;

        drhoEA_bc_darhoA = 0;
        dp_bc_darhoA = 0.5 * u2 / _area[_qp] * (1 - _k_coeff);
      }
      else
      {
        rhoEA_bc = _rhoA[_qp] * (_rhoe_junction[0] / _rho_junction[0] + 0.5 * u2);
        Real pressure_form_loss = _kr_coeff * kinetic_energy_pipe;
        pressure_pipe_bc =
            pressure_junction - pressure_form_loss + pressure_acc_loss + pressure_grav_loss;

        drhoEA_bc_darhoA = (_rhoe_junction[0] / _rho_junction[0] - 0.5 * u2);
        dp_bc_darhoA = 0.5 * u2 / _area[_qp] * (1 + _kr_coeff);
      }

      return (drhoEA_bc_darhoA + dp_bc_darhoA * _area[_qp] -
              (rhoEA_bc + pressure_pipe_bc * _area[_qp]) / _rhoA[_qp]) *
             _normal * _vel[_qp] * _phi[_j][_qp] * _test[_i][_qp];
    }
    else if (jvar == _rhouA_var_number)
    {
      Real rho_junction = _rho_junction[0];
      Real pressure_junction = _pressure_junction[0];
      Real ref_vel = _total_mfr_in[0] / rho_junction / _ref_area;
      Real kinetic_energy_junction = 0.5 * rho_junction * ref_vel * ref_vel;
      Real kinetic_energy_pipe = 0.5 * _rho[_qp] * _vel[_qp] * _vel[_qp];
      Real pressure_acc_loss = kinetic_energy_junction - kinetic_energy_pipe;
      Real pressure_grav_loss = rho_junction * _gravity_magnitude * _deltaH;
      Real pressure_pipe_bc;
      Real rhoEA_bc = 0;
      Real drhoEA_bc_darhouA;
      Real dp_bc_darhouA;
      Real dref_vel_darhouA = 0;
      Real u2 = _vel[_qp] * _vel[_qp];

      if (_normal * _vel[_qp] > 0) // case of net flow into the junction
      {
        rhoEA_bc = _rhoEA[_qp];
        Real pressure_form_loss = _k_coeff * kinetic_energy_pipe;
        pressure_pipe_bc =
            pressure_junction + pressure_form_loss + pressure_acc_loss + pressure_grav_loss;

        drhoEA_bc_darhouA = 0;
        dref_vel_darhouA = _normal / _rho_junction[0] / _ref_area;
        dp_bc_darhouA =
            _vel[_qp] / _area[_qp] * (_k_coeff - 1) + _rho_junction[0] * ref_vel * dref_vel_darhouA;
      }
      else
      {
        rhoEA_bc = _rhoA[_qp] * (_rhoe_junction[0] / _rho_junction[0] + 0.5 * u2);
        Real pressure_form_loss = _kr_coeff * kinetic_energy_pipe;
        pressure_pipe_bc =
            pressure_junction - pressure_form_loss + pressure_acc_loss + pressure_grav_loss;

        drhoEA_bc_darhouA = _vel[_qp];
        // dref_vel_darhouA = 0
        dp_bc_darhouA = _vel[_qp] / _area[_qp] * (-_kr_coeff - 1);
      }

      return ((drhoEA_bc_darhouA + dp_bc_darhouA * _area[_qp]) * _vel[_qp] +
              (rhoEA_bc + pressure_pipe_bc * _area[_qp]) / _rhoA[_qp]) *
             _normal * _phi[_j][_qp] * _test[_i][_qp];
    }
    else if (jvar == _rho_junction_var_number)
    {
      Real rho_junction = _rho_junction[0];
      Real ref_vel = _total_mfr_in[0] / rho_junction / _ref_area;
      Real p, dp_dv, dp_de;
      _fp.p_from_v_e(1. / _rho_junction[0], _rhoe_junction[0] / _rho_junction[0], p, dp_dv, dp_de);
      Real dv_drho_br = -1 / _rho_junction[0] / _rho_junction[0];
      Real de_drho_br = -_rhoe_junction[0] / _rho_junction[0] / _rho_junction[0];
      Real dp_junction_drho_br = dp_dv * dv_drho_br + dp_de * de_drho_br;
      Real dref_vel_drho_br = -_total_mfr_in[0] / _ref_area / _rho_junction[0] / _rho_junction[0];
      Real dp_bc_drho_br =
          dp_junction_drho_br +
          0.5 * (ref_vel * ref_vel + 2. * rho_junction * ref_vel * dref_vel_drho_br) +
          _gravity_magnitude * _deltaH;
      Real drhoEA_bc_drho_br;
      if (_normal * _vel[_qp] > 0) // case of net flow into the junction
        drhoEA_bc_drho_br = 0;
      else
        drhoEA_bc_drho_br = -_rhoA[_qp] * _rhoe_junction[0] / _rho_junction[0] / _rho_junction[0];

      return (drhoEA_bc_drho_br + dp_bc_drho_br * _area[_qp]) * _vel[_qp] * _normal *
             _test[_i][_qp];
    }
    else if (jvar == _rhoe_junction_var_number)
    {
      Real p, dp_dv, dp_de;
      _fp.p_from_v_e(1. / _rho_junction[0], _rhoe_junction[0] / _rho_junction[0], p, dp_dv, dp_de);
      Real de_drhoe_br = 1 / _rho_junction[0];
      Real dp_drhoe_br = dp_de * de_drhoe_br;
      if (_normal * _vel[_qp] > 0) // case of net flow into the junction
        return dp_drhoe_br * _area[_qp] * _vel[_qp] * _normal * _test[_i][_qp];
      else
        return (_rhoA[_qp] / _rho_junction[0] + dp_drhoe_br * _area[_qp]) * _vel[_qp] * _normal *
               _test[_i][_qp];
    }
    else if (jvar == _vel_junction_var_number)
    {
      return 0;
    }
  }

  return 0;
}
