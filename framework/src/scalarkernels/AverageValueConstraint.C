//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AverageValueConstraint.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableScalar.h"

registerMooseObject("MooseApp", AverageValueConstraint);

InputParameters
AverageValueConstraint::validParams()
{
  InputParameters params = ScalarKernel::validParams();
  params.addClassDescription("This class is used to enforce integral of phi with a "
                             "Lagrange multiplier approach.");
  params.addRequiredParam<PostprocessorName>(
      "pp_name", "Name of the Postprocessor value we are trying to equate with 'value'.");
  params.addRequiredParam<Real>(
      "value", "Given (constant) which we want the integral of the solution variable to match.");

  return params;
}

AverageValueConstraint::AverageValueConstraint(const InputParameters & parameters)
  : ScalarKernel(parameters),
    _value(getParam<Real>("value")),
    _pp_value(getPostprocessorValue("pp_name"))
{
}

void
AverageValueConstraint::reinit()
{
}

Real
AverageValueConstraint::computeQpResidual()
{
  return _pp_value - _value;
}

Real
AverageValueConstraint::computeQpJacobian()
{
  // Note: Here, the true on-diagonal Jacobian contribution is
  // actually zero, i.e. we are not making any approximation
  // here. That is because the "lambda"-equation in this system of
  // equations does not depend on lambda. For more information, see
  // the detailed writeup [0].
  //
  // [0]: https://github.com/idaholab/large_media/blob/master/scalar_constraint_kernel.pdf
  return 0.;
}

void
AverageValueConstraint::computeOffDiagJacobianScalar(unsigned int /*jvar*/)
{
}

Real
AverageValueConstraint::computeQpOffDiagJacobianScalar(unsigned int /*jvar*/)
{
  // The off-diagonal contribution for this ScalarKernel (derivative
  // wrt the "primal" field variable) is not _actually_ zero, but we
  // are computing it elsewhere (see ScalarLagrangeMultiplier.C) so
  // here we simply return zero. For more information on this, see the
  // detailed writeup [0].
  //
  // [0]: https://github.com/idaholab/large_media/blob/master/scalar_constraint_kernel.pdf
  return 0.;
}
