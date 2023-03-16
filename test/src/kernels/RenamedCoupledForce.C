//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RenamedCoupledForce.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"

registerMooseObject("MooseTestApp", RenamedCoupledForce);
registerMooseObject("MooseTestApp", ADRenamedCoupledForce);

template <bool is_ad>
InputParameters
RenamedCoupledForceTempl<is_ad>::validParams()
{
  InputParameters params = CoupledForceTempl<is_ad>::validParams();
  params.renameCoupledVar(
      "v", "coupled_force_variable", "The variable providing the coupled force.");
  return params;
}

template <bool is_ad>
RenamedCoupledForceTempl<is_ad>::RenamedCoupledForceTempl(const InputParameters & parameters)
  : CoupledForceTempl<is_ad>(parameters)
{
}

template class RenamedCoupledForceTempl<false>;
template class RenamedCoupledForceTempl<true>;
