//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NaNInterface.h"
#include "MooseObject.h"

template <>
InputParameters
validParams<NaNInterface>()
{
  InputParameters params = emptyInputParameters();

  params.addParam<bool>("use_quiet_nans", false, "Use quiet NaNs instead of signaling NaNs?");

  return params;
}

NaNInterface::NaNInterface(const MooseObject * moose_object)
  : _use_quiet_nans(moose_object->getParam<bool>("use_quiet_nans"))
{
}

Real
NaNInterface::getNaN() const
{
  if (_use_quiet_nans)
    return std::nan("");
  else
    /*
    It was found that a number of potentially signalling NaNs did not actually
    signal, at least with a certain OS/hardware/compiler configuration. For
    example, the following did not signal:
      std::numeric_limits<Real>::signaling_NaN()
      1 * std::numeric_limits<Real>::infinity()
      0.0 / 0.0
      1.0 / 0.0
      0 * (0.0 / 0.0)
      0 * (1.0 / 0.0)
    */
    return 0 * std::numeric_limits<Real>::infinity();
}
