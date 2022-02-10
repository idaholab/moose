//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RDG3EqnMaterial.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("ThermalHydraulicsApp", RDG3EqnMaterial);

InputParameters
RDG3EqnMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params += SlopeReconstruction1DInterface<false>::validParams();

  params.addClassDescription(
      "Reconstructed solution values for the 1-D, 1-phase, variable-area Euler equations");

  params.addRequiredCoupledVar("A_elem", "Cross-sectional area, elemental");
  params.addRequiredCoupledVar("A_linear", "Cross-sectional area, linear");
  params.addRequiredCoupledVar("rhoA", "Conserved variable: rho*A");
  params.addRequiredCoupledVar("rhouA", "Conserved variable: rho*u*A");
  params.addRequiredCoupledVar("rhoEA", "Conserved variable: rho*E*A");

  params.addRequiredParam<MaterialPropertyName>("direction",
                                                "Flow channel direction material property name");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name of fluid properties user object");

  return params;
}

RDG3EqnMaterial::RDG3EqnMaterial(const InputParameters & parameters)
  : Material(parameters),
    SlopeReconstruction1DInterface<false>(this),

    _A_avg(coupledValue("A_elem")),
    _A_linear(coupledValue("A_linear")),
    _rhoA_avg(coupledValue("rhoA")),
    _rhouA_avg(coupledValue("rhouA")),
    _rhoEA_avg(coupledValue("rhoEA")),

    _A_var(getVar("A_elem", 0)),
    _rhoA_var(getVar("rhoA", 0)),
    _rhouA_var(getVar("rhouA", 0)),
    _rhoEA_var(getVar("rhoEA", 0)),

    _dir(getMaterialProperty<RealVectorValue>("direction")),

    _rhoA(declareProperty<Real>("rhoA")),
    _rhouA(declareProperty<Real>("rhouA")),
    _rhoEA(declareProperty<Real>("rhoEA")),

    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

void
RDG3EqnMaterial::computeQpProperties()
{
  // Get the limited slopes of the primitive variables: {p, u, T}.
  const auto slopes = getElementSlopes(_current_elem);
  const Real p_slope = slopes[PRESSURE];
  const Real vel_slope = slopes[VELOCITY];
  const Real T_slope = slopes[TEMPERATURE];

  // compute primitive variables from the cell-average solution
  const Real rho_avg = _rhoA_avg[_qp] / _A_avg[_qp];
  const Real vel_avg = _rhouA_avg[_qp] / _rhoA_avg[_qp];
  const Real v_avg = 1.0 / rho_avg;
  const Real e_avg = _rhoEA_avg[_qp] / _rhoA_avg[_qp] - 0.5 * vel_avg * vel_avg;
  const Real p_avg = _fp.p_from_v_e(v_avg, e_avg);
  const Real T_avg = _fp.T_from_v_e(v_avg, e_avg);

  // apply slopes to primitive variables
  const Real delta_x = (_q_point[_qp] - _current_elem->vertex_average()) * _dir[_qp];
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

std::vector<Real>
RDG3EqnMaterial::computeElementPrimitiveVariables(const Elem * elem) const
{
  // get the cell-average conserved variables
  Real A, rhoA, rhouA, rhoEA;
  if (_is_implicit)
  {
    A = _A_var->getElementalValue(elem);
    rhoA = _rhoA_var->getElementalValue(elem);
    rhouA = _rhouA_var->getElementalValue(elem);
    rhoEA = _rhoEA_var->getElementalValue(elem);
  }
  else
  {
    A = _A_var->getElementalValueOld(elem);
    rhoA = _rhoA_var->getElementalValueOld(elem);
    rhouA = _rhouA_var->getElementalValueOld(elem);
    rhoEA = _rhoEA_var->getElementalValueOld(elem);
  }

  // compute primitive variables

  const Real rho = rhoA / A;
  const Real vel = rhouA / rhoA;
  const Real v = 1.0 / rho;
  const Real e = rhoEA / rhoA - 0.5 * vel * vel;

  std::vector<Real> W(_n_slopes);
  W[PRESSURE] = _fp.p_from_v_e(v, e);
  W[VELOCITY] = vel;
  W[TEMPERATURE] = _fp.T_from_v_e(v, e);

  return W;
}
