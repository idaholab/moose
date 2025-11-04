//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RelativeSolutionDifferenceNorm.h"
#include "TransientBase.h"

registerMooseObject("MooseApp", RelativeSolutionDifferenceNorm);

InputParameters
RelativeSolutionDifferenceNorm::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  params.addClassDescription(
      "Computes the relative norm of the solution difference of two consecutive time steps.");

  params.addParam<bool>(
      "use_aux",
      "If true, use the auxiliary system variables for the norm instead of the solution variables. "
      "If false, use the solution system variables only. If not provided, the executioner's value "
      "for the 'check_aux' parameter is used.");

  return params;
}

RelativeSolutionDifferenceNorm::RelativeSolutionDifferenceNorm(const InputParameters & params)
  : GeneralPostprocessor(params),
    _trex(dynamic_cast<TransientBase *>(_app.getExecutioner())),
    _use_aux(isParamValid("use_aux") ? getParam<bool>("use_aux")
                                     : _trex->getParam<bool>("check_aux"))
{
  if (!_trex)
    mooseError("RelativeSolutionDifferenceNorm postprocessor is only for transient calculations");
}

Real
RelativeSolutionDifferenceNorm::getValue() const
{
  return _trex->relativeSolutionDifferenceNorm(_use_aux);
}
