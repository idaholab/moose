//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosNodalVariableStatistics.h"

registerKokkosUserObject("MooseApp", KokkosNodalVariableStatistics);

InputParameters
KokkosNodalVariableStatistics::validParams()
{
  InputParameters params = KokkosNodalStatistics::validParams();

  params.addRequiredCoupledVar("coupled_var", "Coupled variable whose value is used.");

  params.addClassDescription("Nodal reporter to get statistics for a coupled variable. This can "
                             "be transfered to other apps.");
  return params;
}

KokkosNodalVariableStatistics::KokkosNodalVariableStatistics(const InputParameters & parameters)
  : KokkosNodalStatistics(parameters), _v(kokkosCoupledValue("coupled_var"))
{
}
