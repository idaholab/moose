//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SinglePhaseFluidPropertiesPT.h"

template <>
InputParameters
validParams<SinglePhaseFluidPropertiesPT>()
{
  InputParameters params = validParams<SinglePhaseFluidProperties>();

  return params;
}

SinglePhaseFluidPropertiesPT::SinglePhaseFluidPropertiesPT(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters)
{
  mooseDeprecated(name(),
                  ": Your fluid property class inherits from SinglePhaseFluidPropertiesPT which is "
                  "now depreacted. Please, update your code so that your class inherits from "
                  "SinglePhaseFluidProperties.");
}

SinglePhaseFluidPropertiesPT::~SinglePhaseFluidPropertiesPT() {}
Real SinglePhaseFluidPropertiesPT::p_from_v_e(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void SinglePhaseFluidPropertiesPT::p_from_v_e(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidPropertiesPT::T_from_v_e(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void SinglePhaseFluidPropertiesPT::T_from_v_e(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidPropertiesPT::c_from_v_e(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void SinglePhaseFluidPropertiesPT::c_from_v_e(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidPropertiesPT::cp_from_v_e(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidPropertiesPT::cv_from_v_e(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidPropertiesPT::mu_from_v_e(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidPropertiesPT::k_from_v_e(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidPropertiesPT::s_from_v_e(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void SinglePhaseFluidPropertiesPT::s_from_v_e(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidPropertiesPT::s_from_h_p(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void SinglePhaseFluidPropertiesPT::s_from_h_p(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidPropertiesPT::rho_from_p_s(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void SinglePhaseFluidPropertiesPT::rho_from_p_s(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidPropertiesPT::e_from_v_h(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void SinglePhaseFluidPropertiesPT::e_from_v_h(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidPropertiesPT::rho_from_p_T(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void SinglePhaseFluidPropertiesPT::rho_from_p_T(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidPropertiesPT::e_from_p_rho(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void SinglePhaseFluidPropertiesPT::e_from_p_rho(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidPropertiesPT::h_from_p_T(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void SinglePhaseFluidPropertiesPT::h_from_p_T(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidPropertiesPT::p_from_h_s(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

void SinglePhaseFluidPropertiesPT::p_from_h_s(Real, Real, Real &, Real &, Real &) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}

Real SinglePhaseFluidPropertiesPT::g_from_v_e(Real, Real) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}
