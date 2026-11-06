//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosCopyValueAux.h"

registerKokkosAuxKernel("MooseApp", KokkosCopyValueAux);

InputParameters
KokkosCopyValueAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Returns the specified variable as an auxiliary variable with a "
                             "simple copy of the variable values.");
  params.addRequiredCoupledVar(
      "source",
      "Variable to take the value of, or one variable per auxiliary variable in 'variables'.");
  MooseEnum stateEnum("CURRENT=0 OLD=1 OLDER=2", "CURRENT");
  params.addParam<MooseEnum>(
      "state",
      stateEnum,
      "This parameter specifies the state being copied. CURRENT=0 OLD=1 OLDER=2. Copying an older "
      "state allows access to previous solution information if necessary.");
  return params;
}

KokkosCopyValueAux::KokkosCopyValueAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _state(getParam<MooseEnum>("state")),
    _v(_state == 0   ? kokkosCoupledDofValues("source")
       : _state == 1 ? kokkosCoupledDofValuesOld("source")
                     : kokkosCoupledDofValuesOlder("source"))
{
  const auto var_param_name = parameters.varParamName();
  const auto num_vars = getVariableNamesHelper(parameters).size();

  if (coupledComponents("source") != num_vars)
    paramError("source",
               "The number of source variables (",
               coupledComponents("source"),
               ") should match the number of auxiliary variables (",
               num_vars,
               ").");

  // All auxiliary variables have the same FE type, so each source is checked against the first
  for (const auto comp : make_range(num_vars))
  {
    const auto & source_variable = *getVarHelper<MooseVariableBase>("source", comp);

    if (_var.feType().family != source_variable.feType().family)
      paramError(var_param_name,
                 "Source (" + Moose::stringify(source_variable.feType().family) + ") and target (" +
                     Moose::stringify(_var.feType().family) +
                     ") variable have different families. You may use a ProjectionAux "
                     "instead of the CopyValueAux");
    if (_var.feType().order != source_variable.feType().order)
      paramError(var_param_name,
                 "Source (" + Moose::stringify(source_variable.feType().order) + ") and target (" +
                     Moose::stringify(_var.feType().order) +
                     ") variable are of different orders. You may use a ProjectionAux "
                     "instead of the CopyValueAux");
  }
}
