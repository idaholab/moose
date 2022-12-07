//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MDFluidMassKernel.h"
#include "MooseMesh.h"

registerMooseObject("NavierStokesApp", MDFluidMassKernel);

InputParameters
MDFluidMassKernel::validParams()
{
  InputParameters params = MDFluidKernelStabilization::validParams();
  return params;
}

MDFluidMassKernel::MDFluidMassKernel(const InputParameters & parameters)
  : MDFluidKernelStabilization(parameters),
    _u_vel_second(_u_var.secondSln()),
    _v_vel_second(_v_var.secondSln()),
    _w_vel_second(_mesh.dimension() == 3 ? _w_var.secondSln() : _second_zero)
{
}

Real
MDFluidMassKernel::computeQpResidual()
{
  RealVectorValue vec_vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  Real masseq_part = -_rho[_qp] * vec_vel * _grad_test[_i][_qp];

  // Compute PSPG term due to momentum equation residual vector
  Real porosity = _has_porosity ? _porosity[_qp] : 1.0;
  RealVectorValue convection_vec(
      vec_vel * _grad_u_vel[_qp], vec_vel * _grad_v_vel[_qp], vec_vel * _grad_w_vel[_qp]);
  RealVectorValue psi_pspg = _tauc[_qp] * _grad_test[_i][_qp];

  Real transient_pspg = _bTransient ? _rho[_qp] * velocityDot() * psi_pspg : 0;
  Real convection_pspg = _rho[_qp] / porosity * convection_vec * psi_pspg;
  Real pressure_pspg = porosity * _grad_u[_qp] * psi_pspg;
  Real gravity_pspg = -porosity * _rho[_qp] * _vec_g * psi_pspg;

  Real viscous_pspg = 0;
  Real pm_friction_pspg = 0;
  if (porosity > 0.99)
  {
    RealVectorValue vec_vel_second(
        _u_vel_second[_qp].tr(), _v_vel_second[_qp].tr(), _w_vel_second[_qp].tr());
    viscous_pspg =
        -(_dynamic_viscosity[_qp] + _turbulence_viscosity[_qp]) * vec_vel_second * psi_pspg;
  }
  else
  {
    Real velmag = std::sqrt(_u_vel[_qp] * _u_vel[_qp] + _v_vel[_qp] * _v_vel[_qp] +
                            _w_vel[_qp] * _w_vel[_qp]);

    pm_friction_pspg += _inertia_resistance_coeff[_qp] * vec_vel * velmag * psi_pspg;
    pm_friction_pspg += _viscous_resistance_coeff[_qp] * vec_vel * psi_pspg;
  }

  // Assemble PSPG terms
  Real momeq_part = transient_pspg + convection_pspg + pressure_pspg + viscous_pspg + gravity_pspg +
                    pm_friction_pspg;

  // Assemble final residual
  return masseq_part + momeq_part;
}

Real
MDFluidMassKernel::computeQpJacobian()
{
  Real porosity = _has_porosity ? _porosity[_qp] : 1.0;
  return porosity * _tauc[_qp] * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}

Real
MDFluidMassKernel::computeQpOffDiagJacobian(unsigned int jvar)
{
  // Convert the Moose numbering to internal porous medium model variable numbering.
  unsigned m = this->map_var_number(jvar);

  switch (m)
  {
    case 1:
    case 2:
    case 3:
    {
      RealVectorValue vec_vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
      Real velmag = std::sqrt(_u_vel[_qp] * _u_vel[_qp] + _v_vel[_qp] * _v_vel[_qp] +
                              _w_vel[_qp] * _w_vel[_qp]);

      Real mass_eqn_part = -_rho[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp](m - 1);

      Real pm_inertial_pspg = 0;
      Real pm_viscous_pspg = 0;
      if (velmag < 1e-3)
        pm_inertial_pspg = 0.;
      else
        pm_inertial_pspg = _inertia_resistance_coeff[_qp](m - 1, m - 1) *
                           (velmag + vec_vel(m - 1) * vec_vel(m - 1) / velmag) * _phi[_j][_qp] *
                           _tauc[_qp] * _grad_test[_i][_qp](m - 1);
      pm_viscous_pspg = _viscous_resistance_coeff[_qp](m - 1, m - 1) * _phi[_j][_qp] * _tauc[_qp] *
                        _grad_test[_i][_qp](m - 1);

      return mass_eqn_part + pm_inertial_pspg + pm_viscous_pspg;
    }

    default:
      return 0;
  }
}
