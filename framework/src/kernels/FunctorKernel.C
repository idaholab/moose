//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorKernel.h"

registerMooseObject("MooseApp", FunctorKernel);

InputParameters
FunctorKernel::validParams()
{
  InputParameters params = ADKernelValue::validParams();

  params.addClassDescription("Adds a term from a functor.");

  params.addRequiredParam<MooseFunctorName>("functor", "Functor to add");
  params.addRequiredParam<bool>(
      "functor_on_rhs",
      "If true, the functor to add is on the right hand side of the equation. By convention, all "
      "terms are moved to the left hand side, so if true, a factor of -1 is applied.");

  MooseEnum modes("add=0 target=1", "add");
  params.addParam<MooseEnum>("mode",
                             modes,
                             "The operation mode, 'add' just returns the value of the functor and "
                             "therefore adds the functor value to the residual"
                             "'target' sets the residual term in such a way that the residual of "
                             "the variable u vanishes if its value matches the given functor. "
                             "Please note: In mode 'target' this kernel only imposes the equality "
                             "between the variable and the functor when it is the only kernel in "
                             "that variable's equation on the specified subdomains.");

  return params;
}

FunctorKernel::FunctorKernel(const InputParameters & parameters)
  : ADKernelValue(parameters),
    _mode((Mode)(int)parameters.get<MooseEnum>("mode")),
    _functor(getFunctor<ADReal>("functor")),
    _sign(getParam<bool>("functor_on_rhs") ? -1.0 : 1.0)
{
}

ADReal
FunctorKernel::precomputeQpResidual()
{
  const Moose::ElemQpArg space_arg = {_current_elem, _qp, _qrule, _q_point[_qp]};
  const auto functor_value = _functor(space_arg, Moose::currentState());

  switch (_mode)
  {
    case Mode::ADD:
      return _sign * functor_value;

    case Mode::TARGET:
      return _sign * (functor_value - _u[_qp]);

    default:
      mooseError("FunctorKernel: Invalid mode");
  }
}
