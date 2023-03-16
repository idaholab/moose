//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFEFluidMomentumKernel.h"

registerMooseObject("NavierStokesApp", INSFEFluidMomentumKernel);
registerMooseObjectRenamed("NavierStokesApp",
                           MDFluidMomentumKernel,
                           "02/01/2024 00:00",
                           INSFEFluidMomentumKernel);

InputParameters
INSFEFluidMomentumKernel::validParams()
{
  InputParameters params = INSFEFluidKernelStabilization::validParams();
  params.addClassDescription("Adds advection, viscous, pressure, friction, and gravity terms to "
                             "the Navier-Stokes momentum equation, potentially with stabilization");
  params.addParam<bool>("conservative_form", false, "Whether conservative form is used");
  params.addParam<bool>(
      "p_int_by_parts", false, "Whether integration by parts is applied to the pressure term");
  params.addRequiredParam<unsigned>("component", "0,1,or 2 for x-, y-, or z- direction");
  return params;
}

INSFEFluidMomentumKernel::INSFEFluidMomentumKernel(const InputParameters & parameters)
  : INSFEFluidKernelStabilization(parameters),
    _grad_eps(coupledGradient("porosity")),
    _conservative_form(getParam<bool>("conservative_form")),
    _p_int_by_parts(getParam<bool>("p_int_by_parts")),
    _component(getParam<unsigned>("component"))
{
}

Real
INSFEFluidMomentumKernel::computeQpResidual()
{
  Real porosity = _has_porosity ? _porosity[_qp] : 1.0;
  RealVectorValue vec_vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // Weak form residual of the momentum equation
  Real convection_part = _conservative_form
                             ? -_rho[_qp] / porosity * _u[_qp] * vec_vel * _grad_test[_i][_qp]
                             : _rho[_qp] / porosity * vec_vel * _grad_u[_qp] * _test[_i][_qp];
  Real gravity_part = -porosity * _rho[_qp] * _vec_g(_component) * _test[_i][_qp];
  Real pressure_part = _p_int_by_parts
                           ? -porosity * _pressure[_qp] * _grad_test[_i][_qp](_component) -
                                 _pressure[_qp] * _grad_eps[_qp](_component) * _test[_i][_qp]
                           : porosity * _grad_pressure[_qp](_component) * _test[_i][_qp];

  Real viscous_part = 0;
  Real friction = 0;
  Real friction_part = 0;
  if (porosity > 0.99)
  {
    RealVectorValue viscous_stress_vec(_viscous_stress_tensor[_qp](_component, 0),
                                       _viscous_stress_tensor[_qp](_component, 1),
                                       _viscous_stress_tensor[_qp](_component, 2));
    viscous_part = viscous_stress_vec * _grad_test[_i][_qp];
  }
  else
  {
    Real velmag = std::sqrt(_u_vel[_qp] * _u_vel[_qp] + _v_vel[_qp] * _v_vel[_qp] +
                            _w_vel[_qp] * _w_vel[_qp]);
    Real pm_inertial_part =
        _inertia_resistance_coeff[_qp](_component, _component) * vec_vel(_component) * velmag;
    Real pm_viscous_part =
        _viscous_resistance_coeff[_qp](_component, _component) * vec_vel(_component);
    friction = pm_inertial_part + pm_viscous_part;
    friction_part = friction * _test[_i][_qp];
  }

  // Assemble weak form terms
  Real normal_part = convection_part + pressure_part + gravity_part + viscous_part + friction_part;

  // SUPG contributions
  Real psi_supg = _taum[_qp] * _vel_elem * _grad_test[_i][_qp];

  Real transient_supg = _bTransient ? _rho[_qp] * velocityDot()(_component) : 0;
  Real convection_supg = _rho[_qp] / porosity * vec_vel * _grad_u[_qp];
  Real pressure_supg = porosity * _grad_pressure[_qp](_component);
  Real gravity_supg = -porosity * _rho[_qp] * _vec_g(_component);
  Real viscous_supg = 0;
  Real friction_supg = friction;
  if (porosity > 0.99)
    viscous_supg = -(_dynamic_viscosity[_qp] + _turbulence_viscosity[_qp]) * _second_u[_qp].tr();

  // Assemble SUPG terms
  Real supg_part = psi_supg * (transient_supg + convection_supg + pressure_supg + viscous_supg +
                               friction_supg + gravity_supg);

  // Assemble weak form and SUPG contributions
  Real res = normal_part + supg_part;
  if (_p_int_by_parts && (_component == 0) &&
      (_fe_problem.getCoordSystem(_current_elem->subdomain_id()) == Moose::COORD_RZ))
    res -= porosity * _pressure[_qp] / _q_point[_qp](0) * _test[_i][_qp];

  return res;
}

Real
INSFEFluidMomentumKernel::computeQpJacobian()
{
  Real porosity = _has_porosity ? _porosity[_qp] : 1.0;
  RealVectorValue vec_vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // supg parts (some terms can be reused in the weak form)
  Real psi_supg = _taum[_qp] * _vel_elem * _grad_test[_i][_qp];
  Real transient_supg = _bTransient ? _rho[_qp] * _du_dot_du[_qp] * _phi[_j][_qp] : 0;
  Real convection_supg = _rho[_qp] / porosity *
                         (_phi[_j][_qp] * _grad_u[_qp](_component) + _grad_phi[_j][_qp] * vec_vel);

  // Weak form parts
  Real convection_part = 0;
  if (_conservative_form)
    convection_part = -_rho[_qp] / porosity * _phi[_j][_qp] *
                      (vec_vel * _grad_test[_i][_qp] + _u[_qp] * _grad_test[_i][_qp](_component));
  else
    convection_part = convection_supg * _test[_i][_qp];

  Real viscous_part = 0;
  Real friction_part = 0;
  Real friction_supg = 0;
  if (porosity > 0.99)
    viscous_part = (_dynamic_viscosity[_qp] + _turbulence_viscosity[_qp]) *
                   _grad_phi[_j][_qp](_component) * _grad_test[_i][_qp](_component);
  else
  {
    Real velmag = std::sqrt(_u_vel[_qp] * _u_vel[_qp] + _v_vel[_qp] * _v_vel[_qp] +
                            _w_vel[_qp] * _w_vel[_qp]);

    Real pm_inertial_part = 0;
    if (velmag > 1e-3)
      pm_inertial_part = _inertia_resistance_coeff[_qp](_component, _component) * _phi[_j][_qp] *
                         (velmag + vec_vel(_component) * vec_vel(_component) / velmag);
    Real pm_viscous_part = _viscous_resistance_coeff[_qp](_component, _component) * _phi[_j][_qp];

    friction_part = (pm_inertial_part + pm_viscous_part) * _test[_i][_qp];
    friction_supg = (pm_inertial_part + pm_viscous_part) * psi_supg;
  }

  // Assemble weak form and SUPG contributions
  Real normal_part = convection_part + viscous_part + friction_part;
  Real supg_part = psi_supg * (transient_supg + convection_supg) + friction_supg;

  return normal_part + supg_part;
}

Real
INSFEFluidMomentumKernel::computeQpOffDiagJacobian(unsigned int jvar)
{
  unsigned m = this->mapVarNumber(jvar);
  Real porosity = _has_porosity ? _porosity[_qp] : 1.0;
  RealVectorValue vec_vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  Real psi_supg = _taum[_qp] * _vel_elem * _grad_test[_i][_qp];

  Real jac = 0.;
  switch (m)
  {
    case 0:
      jac = _p_int_by_parts ? -porosity * _phi[_j][_qp] * _grad_test[_i][_qp](_component) -
                                  _phi[_j][_qp] * _grad_eps[_qp](_component) * _test[_i][_qp]
                            : porosity * _grad_phi[_j][_qp](_component) * _test[_i][_qp];
      if (_p_int_by_parts && (_component == 0) &&
          (_fe_problem.getCoordSystem(_current_elem->subdomain_id()) == Moose::COORD_RZ))
        jac -= porosity * _phi[_j][_qp] / _q_point[_qp](0) * _test[_i][_qp];
      break;
    case 1:
    case 2:
    case 3:
      if (m != (_component + 1))
      {
        // convection term weak form
        if (_conservative_form)
          jac = -_rho[_qp] / porosity * _phi[_j][_qp] * _u[_qp] * _grad_test[_i][_qp](m - 1);
        else
          jac = _rho[_qp] / porosity * _phi[_j][_qp] * _grad_u[_qp](m - 1) * _test[_i][_qp];
        // convection SUPG term
        jac += _rho[_qp] / porosity * _phi[_j][_qp] * _grad_u[_qp](m - 1) * psi_supg;
        // pm friction
        if (porosity <= 0.99)
        {
          Real velmag = std::sqrt(_u_vel[_qp] * _u_vel[_qp] + _v_vel[_qp] * _v_vel[_qp] +
                                  _w_vel[_qp] * _w_vel[_qp]);
          if (velmag > 1e-3)
          {
            // inertia_resistance, both normal and supg parts
            jac += _inertia_resistance_coeff[_qp](_component, _component) * _phi[_j][_qp] *
                   vec_vel(_component) * vec_vel(m - 1) / velmag * (_test[_i][_qp] + psi_supg);
          }
        }
      }
      break;

    case 4:
    default:
      jac = 0.;
  }
  return jac;
}
