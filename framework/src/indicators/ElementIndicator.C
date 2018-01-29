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
#include "MooseVariable.h"
#include "SystemBase.h"

#include "libmesh/threads.h"

template <>
InputParameters
validParams<ElementIndicator>()
{
  InputParameters params = validParams<Indicator>();
  params += validParams<MaterialPropertyInterface>();
  params.addRequiredParam<VariableName>("variable",
                                        "The name of the variable that this Indicator operates on");

  std::vector<SubdomainName> everywhere(1, "ANY_BLOCK_ID");
  params.addParam<std::vector<SubdomainName>>(
      "block", everywhere, "block ID or name where the object works");

  params += validParams<TransientInterface>();
  return params;
}

ElementIndicator::ElementIndicator(const InputParameters & parameters)
  : Indicator(parameters),
    TransientInterface(this),
    PostprocessorInterface(this),
    Coupleable(this, false),
    ScalarCoupleable(this),
    MooseVariableInterface(this, false),

    _field_var(_sys.getVariable(_tid, name())),

    _current_elem(_field_var.currentElem()),
    _current_elem_volume(_assembly.elemVolume()),
    _q_point(_assembly.qPoints()),
    _qrule(_assembly.qRule()),
    _JxW(_assembly.JxW()),
    _coord(_assembly.coordTransformation()),

    _var(_subproblem.getVariable(_tid, parameters.get<VariableName>("variable"))),

    _u(_var.sln()),
    _grad_u(_var.gradSln()),
    _u_dot(_var.uDot()),
    _du_dot_du(_var.duDotDu())
{
  const std::vector<MooseVariable *> & coupled_vars = getCoupledMooseVars();
  for (const auto & var : coupled_vars)
    addMooseVariableDependency(var);

  addMooseVariableDependency(mooseVariable());
}
