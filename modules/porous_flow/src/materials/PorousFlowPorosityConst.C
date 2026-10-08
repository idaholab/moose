//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPorosityConst.h"
#include "Conversion.h"

registerMooseObject("PorousFlowApp", PorousFlowPorosityConst);
registerMooseObject("PorousFlowApp", ADPorousFlowPorosityConst);

template <bool is_ad>
InputParameters
PorousFlowPorosityConstTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowPorosityBaseTempl<is_ad>::validParams();
  params.addRequiredCoupledVar(
      "porosity",
      "The porosity (assumed indepenent of porepressure, temperature, "
      "strain, etc, for this material).  This should be a real number, or "
      "a constant monomial variable (not a linear lagrange or other kind of variable).  It must "
      "not be negative, nor less than porosity_min if porosity_min is provided.");
  params.addClassDescription("This Material calculates the porosity assuming it is constant");
  return params;
}

template <bool is_ad>
PorousFlowPorosityConstTempl<is_ad>::PorousFlowPorosityConstTempl(
    const InputParameters & parameters)
  : PorousFlowPorosityBaseTempl<is_ad>(parameters),
    _input_porosity(coupledValue("porosity")),
    _porosity_is_constant(this->isCoupledConstant("porosity"))
{
}

template <bool is_ad>
void
PorousFlowPorosityConstTempl<is_ad>::initQpStatefulProperties()
{
  // note the [0] below: _phi0 is a constant monomial and we use [0] regardless of _nodal_material
  const Real phi = _input_porosity[0];

  // Porosity never changes in this Material, so a value below the lower bound is an input error,
  // and is reported rather than silently clipped
  const Real lower_bound = std::max(0.0, _porosity_min);
  if (phi < lower_bound)
  {
    const std::string bound =
        (_porosity_min > 0.0) ? "porosity_min (" + Moose::stringify(_porosity_min) + ")" : "zero";
    if (_porosity_is_constant)
      this->paramError("porosity", "The porosity (", phi, ") is less than ", bound);
    else
      this->paramError("porosity",
                       "The porosity variable is less than ",
                       bound,
                       " in element ",
                       this->_current_elem->id(),
                       " (porosity = ",
                       phi,
                       ")");
  }

  _porosity[_qp] = phi;
}

template <bool is_ad>
void
PorousFlowPorosityConstTempl<is_ad>::computeQpProperties()
{
  initQpStatefulProperties();

  if (!is_ad)
  {
    // The derivatives are zero for all time
    (*_dporosity_dvar)[_qp].assign(_num_var, 0.0);
    (*_dporosity_dgradvar)[_qp].assign(_num_var, RealGradient());
  }
  // porosity_min is checked in initQpStatefulProperties (porosity is constant, so it is never
  // clipped), which means the current and old porosity are always identical
}

template class PorousFlowPorosityConstTempl<false>;
template class PorousFlowPorosityConstTempl<true>;
