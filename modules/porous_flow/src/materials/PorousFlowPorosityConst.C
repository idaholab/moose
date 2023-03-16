//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPorosityConst.h"

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
      "a constant monomial variable (not a linear lagrange or other kind of variable).");
  params.addClassDescription("This Material calculates the porosity assuming it is constant");
  return params;
}

template <bool is_ad>
PorousFlowPorosityConstTempl<is_ad>::PorousFlowPorosityConstTempl(
    const InputParameters & parameters)
  : PorousFlowPorosityBaseTempl<is_ad>(parameters), _input_porosity(coupledValue("porosity"))
{
}

template <bool is_ad>
void
PorousFlowPorosityConstTempl<is_ad>::initQpStatefulProperties()
{
  // note the [0] below: _phi0 is a constant monomial and we use [0] regardless of _nodal_material
  _porosity[_qp] = _input_porosity[0];
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
}

template class PorousFlowPorosityConstTempl<false>;
template class PorousFlowPorosityConstTempl<true>;
