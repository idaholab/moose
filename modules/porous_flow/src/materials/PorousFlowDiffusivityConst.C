/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowDiffusivityConst.h"

template <>
InputParameters
validParams<PorousFlowDiffusivityConst>()
{
  InputParameters params = validParams<PorousFlowDiffusivityBase>();
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
    mooseError("The number of tortuosity values entered is not equal to the number of phases "
               "specified in the Dictator");
}

void
PorousFlowDiffusivityConst::computeQpProperties()
{
  PorousFlowDiffusivityBase::computeQpProperties();

  _tortuosity[_qp] = _input_tortuosity;
}
