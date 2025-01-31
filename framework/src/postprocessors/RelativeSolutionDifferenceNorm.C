//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

  return params;
}

RelativeSolutionDifferenceNorm::RelativeSolutionDifferenceNorm(const InputParameters & params)
  : GeneralPostprocessor(params), _trex(dynamic_cast<TransientBase *>(_app.getExecutioner()))
{
  if (!_trex)
    mooseError("RelativeSolutionDifferenceNorm postprocessor is only for transient calculations");
}

Real
RelativeSolutionDifferenceNorm::getValue() const
{
  return _trex->relativeSolutionDifferenceNorm();
}
