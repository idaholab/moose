//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BondStatusBasePD.h"
#include "PeridynamicsMesh.h"

InputParameters
BondStatusBasePD::validParams()
{
  InputParameters params = AuxKernelBasePD::validParams();
  params.addClassDescription("Base class for different failure criteria to update the bond status");

  params.addRequiredCoupledVar("critical_variable", "Name of critical AuxVariable");

  params.set<ExecFlagEnum>("execute_on", true) = EXEC_TIMESTEP_END;

  return params;
}

BondStatusBasePD::BondStatusBasePD(const InputParameters & parameters)
  : AuxKernelBasePD(parameters),
    _bond_status_var(&_subproblem.getStandardVariable(_tid, "bond_status")),
    _critical_val(coupledValue("critical_variable"))
{
  if (isNodal())
    paramError("variable", "This AuxKernel only supports Elemental fields");
}

Real
BondStatusBasePD::computeValue()
{
  Real val = 0.0;

  if (_bond_status_var->getElementalValue(_current_elem) > 0.5) // unbroken bond
  {
    Real failure_criterion_val = computeFailureCriterionValue();
    if (failure_criterion_val < 0.0) // unmet failure criterion
      val = 1.0;                     // bond is still unbroken
  }

  return val;
}
