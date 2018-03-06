//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SinglePhaseFluidProperties.h"

template <>
InputParameters
validParams<SinglePhaseFluidProperties>()
{
  InputParameters params = validParams<FluidProperties>();

  return params;
}

SinglePhaseFluidProperties::SinglePhaseFluidProperties(const InputParameters & parameters)
  : FluidProperties(parameters)
{
}

SinglePhaseFluidProperties::~SinglePhaseFluidProperties() {}

Real SinglePhaseFluidProperties::p_from_v_e(Real, Real) const { mooseError("Not implemented"); }

void
SinglePhaseFluidProperties::p_from_v_e(Real, Real, Real &, Real &, Real &) const
{
  mooseError("Not implemented");
}

Real SinglePhaseFluidProperties::T_from_v_e(Real, Real) const { mooseError("Not implemented"); }

void
SinglePhaseFluidProperties::T_from_v_e(Real, Real, Real &, Real &, Real &) const
{
  mooseError("Not implemented");
}

Real SinglePhaseFluidProperties::c_from_v_e(Real, Real) const { mooseError("Not implemented"); }

void
SinglePhaseFluidProperties::c_from_v_e(Real, Real, Real &, Real &, Real &) const
{
  mooseError("Not implemented");
}

Real SinglePhaseFluidProperties::cp_from_v_e(Real, Real) const { mooseError("Not implemented"); }

Real SinglePhaseFluidProperties::cv_from_v_e(Real, Real) const { mooseError("Not implemented"); }

Real
SinglePhaseFluidProperties::gamma_from_v_e(Real v, Real e) const
{
  return cp_from_v_e(v, e) / cv_from_v_e(v, e);
}

Real SinglePhaseFluidProperties::mu_from_v_e(Real, Real) const { mooseError("Not implemented"); }

Real SinglePhaseFluidProperties::k_from_v_e(Real, Real) const { mooseError("Not implemented"); }

Real SinglePhaseFluidProperties::s_from_v_e(Real, Real) const { mooseError("Not implemented"); }

void
SinglePhaseFluidProperties::s_from_v_e(Real, Real, Real &, Real &, Real &) const
{
  mooseError("Not implemented");
}

Real SinglePhaseFluidProperties::s_from_h_p(Real, Real) const { mooseError("Not implemented"); }

void
SinglePhaseFluidProperties::s_from_h_p(Real, Real, Real &, Real &, Real &) const
{
  mooseError("Not implemented");
}

Real SinglePhaseFluidProperties::rho_from_p_s(Real, Real) const { mooseError("Not implemented"); }

void
SinglePhaseFluidProperties::rho_from_p_s(Real, Real, Real &, Real &, Real &) const
{
  mooseError("Not implemented");
}

Real SinglePhaseFluidProperties::rho_from_p_T(Real, Real) const { mooseError("Not implemented"); }

void
SinglePhaseFluidProperties::rho_from_p_T(Real, Real, Real &, Real &, Real &) const
{
  mooseError("Not implemented");
}

Real SinglePhaseFluidProperties::e_from_p_rho(Real, Real) const { mooseError("Not implemented"); }

void
SinglePhaseFluidProperties::e_from_p_rho(Real, Real, Real &, Real &, Real &) const
{
  mooseError("Not implemented");
}

Real SinglePhaseFluidProperties::e_from_v_h(Real, Real) const { mooseError("Not implemented"); }

void
SinglePhaseFluidProperties::e_from_v_h(Real, Real, Real &, Real &, Real &) const
{
  mooseError("Not implemented");
}

Real SinglePhaseFluidProperties::h_from_p_T(Real, Real) const { mooseError("Not implemented"); }

void
SinglePhaseFluidProperties::h_from_p_T(Real, Real, Real &, Real &, Real &) const
{
  mooseError("Not implemented");
}

Real SinglePhaseFluidProperties::p_from_h_s(Real, Real) const { mooseError("Not implemented"); }

void
SinglePhaseFluidProperties::p_from_h_s(Real, Real, Real &, Real &, Real &) const
{
  mooseError("Not implemented");
}

Real SinglePhaseFluidProperties::g_from_v_e(Real, Real) const { mooseError("Not implemented"); }

Real SinglePhaseFluidProperties::beta_from_p_T(Real, Real) const { mooseError("Not implemented"); }
