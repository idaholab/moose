//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NaNInterface.h"
#include "Conversion.h"
#include "MooseEnum.h"

InputParameters
NaNInterface::validParams()
{
#ifdef NDEBUG
  // in opt mode, getNaN() emits neither a warning nor an error by default
  MooseEnum emit_on_nan("none warning error", "none");
#else
  // in dbg mode, getNaN() raises an error by default
  MooseEnum emit_on_nan("none warning error", "error");
#endif

  InputParameters params = emptyInputParameters();

  params.addParam<MooseEnum>("emit_on_nan", emit_on_nan, "Raise mooseWarning or mooseError?");

  return params;
}

NaNInterface::NaNInterface(const MooseObject * moose_object)
  : _moose_object(moose_object),
    _emit_on_nan(
        _moose_object->getParam<MooseEnum>("emit_on_nan").getEnum<NaNInterface::NaNMessage>())
{
}
