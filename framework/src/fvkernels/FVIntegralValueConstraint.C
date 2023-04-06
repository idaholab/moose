//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVIntegralValueConstraint.h"

#include "MooseVariableScalar.h"
#include "MooseVariableFV.h"
#include "Assembly.h"

registerMooseObject("MooseApp", FVIntegralValueConstraint);
registerMooseObjectRenamed("MooseApp",
                           FVScalarLagrangeMultiplier,
                           "06/30/2022 24:00",
                           FVIntegralValueConstraint);

InputParameters
FVIntegralValueConstraint::validParams()
{
  InputParameters params = FVScalarLagrangeMultiplierConstraint::validParams();
  params.addClassDescription("This class is used to enforce integral of phi = volume * phi_0 "
                             "with a Lagrange multiplier approach.");
  params.setDocString("phi0", "What we want the average value of the primal variable to be.");
  return params;
}

FVIntegralValueConstraint::FVIntegralValueConstraint(const InputParameters & parameters)
  : FVScalarLagrangeMultiplierConstraint(parameters)
{
}

ADReal
FVIntegralValueConstraint::computeQpResidual()
{
  return _var(makeElemArg(_current_elem), determineState()) - _phi0;
}
