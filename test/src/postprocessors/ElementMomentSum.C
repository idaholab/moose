/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ElementMomentSum.h"

#include "MooseVariable.h"

template <>
InputParameters
validParams<ElementMomentSum>()
{
  InputParameters params = validParams<ElementIntegralVariablePostprocessor>();
  params.addParam<bool>("use_old", false, "True to coupled old variable");
  return params;
}

ElementMomentSum::ElementMomentSum(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters),
    _elemental_sln(getParam<bool>("use_old") ? coupledSolutionDoFsOld("variable")
                                             : coupledSolutionDoFs("variable"))
{
}

void
ElementMomentSum::execute()
{
  unsigned int ndofs = _elemental_sln.size();
  Real v = 0;
  for (unsigned int i = 0; i < ndofs; ++i)
    v += _elemental_sln(i);
  v /= ndofs;
  v *= _current_elem_volume;

  _integral_value += v;
}
