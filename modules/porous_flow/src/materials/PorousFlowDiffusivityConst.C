//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowDiffusivityConst.h"

registerMooseObject("PorousFlowApp", PorousFlowDiffusivityConst);

InputParameters
PorousFlowDiffusivityConst::validParams()
{
  InputParameters params = PorousFlowDiffusivityBase::validParams();
  params.addRequiredParam<std::vector<Real>>(
      "tortuosity", "List of tortuosities. Order is i) phase 0; ii) phase 1; etc");
  params.addClassDescription(
      "This Material provides constant tortuosity and diffusion coefficients");
  return params;
}

PorousFlowDiffusivityConst::PorousFlowDiffusivityConst(const InputParameters & parameters)
  : PorousFlowDiffusivityBase(parameters),
    _input_tortuosity(getParam<std::vector<Real>>("tortuosity"))
{
  // Check that the number of tortuosities entered is equal to the number of phases
  if (_input_tortuosity.size() != _num_phases)
    paramError("tortuosity",
               "The number of tortuosity values entered is not equal to the number of phases "
               "specified in the Dictator");

  // Check that all tortuosities are (0, 1]
  for (unsigned int i = 0; i < _num_phases; ++i)
    if (_input_tortuosity[i] <= 0.0 || _input_tortuosity[i] > 1)
      paramError("tortuosity",
                 "All tortuosities must be greater than zero and less than (or equal to) one"
                 ".\nNote: the definition of tortuosity used is l/le, where l is the straight line "
                 "length and le is the effective flow length");
}

void
PorousFlowDiffusivityConst::computeQpProperties()
{
  PorousFlowDiffusivityBase::computeQpProperties();

  _tortuosity[_qp] = _input_tortuosity;
}
