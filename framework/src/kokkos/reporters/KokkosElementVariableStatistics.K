//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosElementVariableStatistics.h"

registerKokkosReducer("MooseApp", KokkosElementVariableStatistics);

InputParameters
KokkosElementVariableStatistics::validParams()
{
  InputParameters params = KokkosElementStatistics::validParams();

  params.addRequiredCoupledVar("coupled_var", "Coupled variable whose value is used.");

  params.addClassDescription("Element reporter to get statistics for a coupled variable. This can "
                             "be transfered to other apps.");
  return params;
}

KokkosElementVariableStatistics::KokkosElementVariableStatistics(const InputParameters & parameters)
  : KokkosElementStatistics(parameters), _v(kokkosCoupledValue("coupled_var"))
{
}
