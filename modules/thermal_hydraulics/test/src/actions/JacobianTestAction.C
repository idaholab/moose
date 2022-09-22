//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JacobianTestAction.h"
#include "MooseObjectAction.h"
#include "ActionFactory.h"
#include "ActionWarehouse.h"
#include "MooseEnum.h"
#include "FEProblemBase.h"
#include "Conversion.h"

InputParameters
JacobianTestAction::validParams()
{
  InputParameters params = TestAction::validParams();

  params.addParam<Real>("snes_test_err", 1e-8, "Finite differencing parameter");

  return params;
}

JacobianTestAction::JacobianTestAction(const InputParameters & params)
  : TestAction(params), _snes_test_err(Moose::stringify<Real>(getParam<Real>("snes_test_err")))
{
}

void
JacobianTestAction::addPreconditioner()
{
  const std::string class_name = "SetupPreconditionerAction";
  InputParameters params = _action_factory.getValidParams(class_name);
  params.set<std::string>("type") = "SMP";

  std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
      _action_factory.create(class_name, "smp", params));

  action->getObjectParams().set<bool>("full") = true;
  action->getObjectParams().set<MooseEnum>("solve_type") = "newton";
#if PETSC_VERSION_LESS_THAN(3, 9, 0)
  action->getObjectParams().set<MultiMooseEnum>("petsc_options_iname") =
      "-snes_type -snes_test_err";
  action->getObjectParams().set<std::vector<std::string>>("petsc_options_value") = {"test",
                                                                                    _snes_test_err};
#else
  action->getObjectParams().set<MultiMooseEnum>("petsc_options") = "-snes_test_jacobian";
  action->getObjectParams().set<MultiMooseEnum>("petsc_options_iname") = "-snes_test_err";
  action->getObjectParams().set<std::vector<std::string>>("petsc_options_value") = {_snes_test_err};
#endif

  _awh.addActionBlock(action);
}
