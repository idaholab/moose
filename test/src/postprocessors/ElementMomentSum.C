//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementMomentSum.h"

#include "MooseVariableFE.h"

registerMooseObject("MooseTestApp", ElementMomentSum);

InputParameters
ElementMomentSum::validParams()
{
  InputParameters params = ElementIntegralVariablePostprocessor::validParams();
  params.addParam<bool>("use_old", false, "True to coupled old variable");
  return params;
}

ElementMomentSum::ElementMomentSum(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters),
    _elemental_sln(getParam<bool>("use_old") ? coupledDofValuesOld("variable")
                                             : coupledDofValues("variable"))
{
}

void
ElementMomentSum::execute()
{
  unsigned int ndofs = _elemental_sln.size();
  Real v = 0;
  for (unsigned int i = 0; i < ndofs; ++i)
    v += _elemental_sln[i];
  v /= ndofs;
  v *= _current_elem_volume;

  _integral_value += v;
}
