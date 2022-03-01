//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementIndicator.h"

#include "Assembly.h"
#include "MooseVariableField.h"
#include "SystemBase.h"

#include "libmesh/threads.h"

InputParameters
ElementIndicator::validParams()
{
  InputParameters params = Indicator::validParams();
  params += MaterialPropertyInterface::validParams();
  params += TransientInterface::validParams();
  params.addRequiredParam<VariableName>("variable",
                                        "The name of the variable that this Indicator operates on");
  std::vector<SubdomainName> everywhere(1, "ANY_BLOCK_ID");
  params.addParam<std::vector<SubdomainName>>(
      "block", everywhere, "block ID or name where the object works");
  return params;
}

ElementIndicator::ElementIndicator(const InputParameters & parameters)
  : Indicator(parameters),
    TransientInterface(this),
    PostprocessorInterface(this),
    Coupleable(this, false),
    ScalarCoupleable(this),
    MooseVariableInterface<Real>(this,
                                 false,
                                 "variable",
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _field_var(_subproblem.getStandardVariable(_tid, name())),

    _current_elem(_field_var.currentElem()),
    _current_elem_volume(_assembly.elemVolume()),
    _q_point(_assembly.qPoints()),
    _qrule(_assembly.qRule()),
    _JxW(_assembly.JxW()),
    _coord(_assembly.coordTransformation()),
    _var(mooseVariableField()),
    _u(_var.sln()),
    _grad_u(_var.gradSln())
{
  const std::vector<MooseVariableFieldBase *> & coupled_vars = getCoupledMooseVars();
  for (const auto & var : coupled_vars)
    addMooseVariableDependency(var);

  addMooseVariableDependency(&mooseVariableField());
}
