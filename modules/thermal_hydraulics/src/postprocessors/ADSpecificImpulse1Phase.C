//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADSpecificImpulse1Phase.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"
#include "THMIndicesVACE.h"
#include "ADBoundaryFluxBase.h"

registerMooseObject("ThermalHydraulicsApp", ADSpecificImpulse1Phase);

InputParameters
ADSpecificImpulse1Phase::validParams()
{
  InputParameters params = SidePostprocessor::validParams();
  params.addRequiredParam<Real>("p_exit", "Outlet pressure at nozzle exit");
  params.addRequiredParam<UserObjectName>("fp", "Single-phase fluid properties");
  params.addParam<Real>(
      "bisection_tolerance", 1e-4, "Tolerance for bisection to find h(entropy_in, p_out)");
  params.addParam<unsigned int>(
      "bisection_max_it",
      100,
      "Maximum number of iterations for bisection to find h(entropy_in, p_out)");
  params.addParam<bool>("cumulative",
                        true,
                        "If specific impulse is accumulated over timesteps. If false, then "
                        "instantaneous value is computed");
  params.addCoupledVar("variables", "Single-phase flow variables");
  params.set<std::vector<VariableName>>("variables") = {"rhoA", "rhouA", "rhoEA", "A"};
  params.addClassDescription("Estimates specific impulse from fluid state at a boundary");
  return params;
}

ADSpecificImpulse1Phase::ADSpecificImpulse1Phase(const InputParameters & parameters)
  : SidePostprocessor(parameters),
    _n_components(THMVACE1D::N_FLUX_INPUTS),
    _boundary_name(getParam<std::vector<BoundaryName>>("boundary")[0]),
    _boundary_uo_name(_boundary_name + ":boundary_uo"),
    _boundary_uo(getUserObjectByName<ADBoundaryFluxBase>(_boundary_uo_name)),
    _p_exit(getParam<Real>("p_exit")),
    _H(getADMaterialPropertyByName<Real>("H")),
    _v(getADMaterialPropertyByName<Real>("v")),
    _e(getADMaterialPropertyByName<Real>("e")),
    _T(getADMaterialPropertyByName<Real>("T")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp")),
    _tol(getParam<Real>("bisection_tolerance")),
    _max_nit(getParam<unsigned int>("bisection_max_it")),
    _cumulative(getParam<bool>("cumulative")),
    _accumulated_mass_flow_rate(declareRestartableData<Real>("accumulated_mass_flow_rate", 0)),
    _accumulated_thrust(declareRestartableData<Real>("accumulated_thrust", 0)),
    _value(0.0)
{
  if (_cumulative && !_fe_problem.isTransient())
    paramError("cumulative", "Must be false unless problem is transient");

  for (unsigned int i = 0; i < _n_components; i++)
    _U.push_back(&adCoupledValue("variables", i));
}

void
ADSpecificImpulse1Phase::threadJoin(const UserObject & y)
{
  const ADSpecificImpulse1Phase & pps = static_cast<const ADSpecificImpulse1Phase &>(y);
  _thrust += pps._thrust;
  _mass_flow_rate += pps._mass_flow_rate;
}

void
ADSpecificImpulse1Phase::initialize()
{
  _mass_flow_rate = 0;
  _thrust = 0;
}

Real
ADSpecificImpulse1Phase::getValue() const
{
  return _value;
}

void
ADSpecificImpulse1Phase::finalize()
{
  gatherSum(_thrust);
  gatherSum(_mass_flow_rate);

  if (_cumulative)
  {
    _accumulated_thrust += _dt * _thrust;
    _accumulated_mass_flow_rate += _dt * _mass_flow_rate;
    _value = _accumulated_thrust / _accumulated_mass_flow_rate / THM::gravity_const;
  }
  else
    _value = _thrust / _mass_flow_rate / THM::gravity_const;
}

void
ADSpecificImpulse1Phase::execute()
{
  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
  {
    std::vector<ADReal> U(_n_components);
    for (unsigned int i = 0; i < _n_components; i++)
      U[i] = (*_U[i])[qp];

    const auto & flux = _boundary_uo.getFlux(_current_side, _current_elem->id(), U, _normals[qp]);

    _mass_flow_rate += std::abs(MetaPhysicL::raw_value(flux[0]));

    // get entropy at inlet (= entropy at outlet because process is isentropic)
    Real entropy_in =
        _fp.s_from_v_e(MetaPhysicL::raw_value(_v[qp]), MetaPhysicL::raw_value(_e[qp]));

    // compute outlet enthalpy from entropy at inlet (isentropic flow)
    // and pressure at outlet, need to do bisection because h_from_s_p does not
    // exist, it is better to work with temperature because s(p, T) is available

    // upper bound of temperature is the inlet temperature
    Real T_up = MetaPhysicL::raw_value(_T[qp]);

    // compute entropy associated with T_up
    Real entropy_up = _fp.s_from_p_T(_p_exit, T_up);

    // lower bound for temperature is 0.1 K
    Real T_low = 0.1;

    // compute entropy associated with T_low
    Real entropy_low = _fp.s_from_p_T(_p_exit, T_low);

    // compute the midpoint temperature
    Real T_mid = 0.5 * (T_up + T_low);

    // the current guess for entropy
    Real entropy_mid = _fp.s_from_p_T(_p_exit, T_mid);

    // main bisection loop
    unsigned int nit = 0;
    while (std::abs(1 - entropy_mid / entropy_in) > _tol)
    {
      // check if we are over/underestimating to select either up/down
      if ((entropy_mid - entropy_in > 0) == (entropy_up - entropy_low > 0))
      {
        T_up = T_mid;
        entropy_up = entropy_mid;
      }
      else
      {
        T_low = T_mid;
        entropy_low = entropy_mid;
      }

      // update guess for T_mid
      T_mid = 0.5 * (T_up + T_low);

      // update the guess for entropy
      entropy_mid = _fp.s_from_p_T(_p_exit, T_mid);

      ++nit;

      if (nit == _max_nit)
      {
        mooseDoOnce(mooseWarning("Bisection in ADSpecificImpulse1Phase did not converge"));
        break;
      }
    }

    // the enthalpy evaluated at _p_exit and T_mid
    Real h_exit = _fp.h_from_p_T(_p_exit, T_mid);

    // compute outlet speed
    Real vel_exit = std::sqrt(2.0 * (MetaPhysicL::raw_value(_H[qp]) - h_exit));
    _thrust += std::abs(vel_exit * MetaPhysicL::raw_value(flux[0]));
  }
}
