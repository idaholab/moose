#include "RDG3EqnMaterial.h"
#include "RELAP7Indices3Eqn.h"
#include "MooseMesh.h"

#include "libmesh/quadrature.h"

registerMooseObject("RELAP7App", RDG3EqnMaterial);

template <>
InputParameters
validParams<RDG3EqnMaterial>()
{
  InputParameters params = validParams<Material>();

  params.addClassDescription(
      "Reconstructed solution values for the 1-D, 1-phase, variable-area Euler equations");

  params.addRequiredCoupledVar("A_elem", "Cross-sectional area, elemental");
  params.addRequiredCoupledVar("A_linear", "Cross-sectional area, linear");
  params.addRequiredCoupledVar("rhoA", "Conserved variable: rho*A");
  params.addRequiredCoupledVar("rhouA", "Conserved variable: rho*u*A");
  params.addRequiredCoupledVar("rhoEA", "Conserved variable: rho*E*A");

  params.addRequiredParam<UserObjectName>("slopes_uo", "Name of slopes user object");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name of fluid properties user object");

  return params;
}

RDG3EqnMaterial::RDG3EqnMaterial(const InputParameters & parameters)
  : Material(parameters),

    _A_avg(coupledValue("A_elem")),
    _A_linear(coupledValue("A_linear")),
    _rhoA_avg(coupledValue("rhoA")),
    _rhouA_avg(coupledValue("rhouA")),
    _rhoEA_avg(coupledValue("rhoEA")),
    _rhoA(declareProperty<Real>("rhoA")),
    _rhouA(declareProperty<Real>("rhouA")),
    _rhoEA(declareProperty<Real>("rhoEA")),
    _slopes_uo(getUserObject<RDGSlopes3Eqn>("slopes_uo")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

void
RDG3EqnMaterial::computeQpProperties()
{
  // get the limited slopes of the primitive variables: {p, u, T}
  const auto slopes = _slopes_uo.getElementSlope(_current_elem->id());
  const auto p_slope = slopes[RELAP73Eqn::SLOPE_PRESSURE];
  const auto vel_slope = slopes[RELAP73Eqn::SLOPE_VELOCITY];
  const auto T_slope = slopes[RELAP73Eqn::SLOPE_TEMPERATURE];

  // compute primitive variables from the cell-average solution
  const Real rho_avg = _rhoA_avg[_qp] / _A_avg[_qp];
  const Real vel_avg = _rhouA_avg[_qp] / _rhoA_avg[_qp];
  const Real v_avg = 1.0 / rho_avg;
  const Real e_avg = _rhoEA_avg[_qp] / _rhoA_avg[_qp] - 0.5 * vel_avg * vel_avg;
  const Real p_avg = _fp.p_from_v_e(v_avg, e_avg);
  const Real T_avg = _fp.T_from_v_e(v_avg, e_avg);

  // apply slopes to primitive variables
  const RealGradient delta_x = _q_point[_qp] - _current_elem->centroid();
  const Real p = p_avg + p_slope * delta_x;
  const Real vel = vel_avg + vel_slope * delta_x;
  const Real T = T_avg + T_slope * delta_x;

  // compute reconstructed conserved variables
  const Real rho = _fp.rho_from_p_T(p, T);
  const Real e = _fp.e_from_p_rho(p, rho);
  const Real E = e + 0.5 * vel * vel;

  _rhoA[_qp] = rho * _A_linear[_qp];
  _rhouA[_qp] = _rhoA[_qp] * vel;
  _rhoEA[_qp] = _rhoA[_qp] * E;
}
