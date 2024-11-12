//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AugmentedLagrangianContactProblem.h"

// MOOSE includes
#include "AuxiliarySystem.h"
#include "DisplacedProblem.h"
#include "MooseApp.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "NearestNodeLocator.h"
#include "NonlinearSystem.h"
#include "PenetrationLocator.h"

#include "SystemBase.h"
#include "Assembly.h"
#include "Executioner.h"
#include "AddVariableAction.h"
#include "ConstraintWarehouse.h"
#include "MortarUserObject.h"
#include "AugmentedLagrangeInterface.h"
#include "AugmentedLagrangianContactConvergence.h"
#include "Convergence.h"

registerMooseObject("ContactApp", AugmentedLagrangianContactProblem);
registerMooseObject("ContactApp", AugmentedLagrangianContactFEProblem);

template <class T>
InputParameters
AugmentedLagrangianContactProblemTempl<T>::validParams()
{
  InputParameters params = T::validParams();
  params += AugmentedLagrangianContactProblemInterface::validParams();
  params.addClassDescription("Manages nested solution for augmented Lagrange contact");
  return params;
}

template <class T>
AugmentedLagrangianContactProblemTempl<T>::AugmentedLagrangianContactProblemTempl(
    const InputParameters & params)
  : T(params), AugmentedLagrangianContactProblemInterface(params)
{
}

template <class T>
void
AugmentedLagrangianContactProblemTempl<T>::timestepSetup()
{
  _lagrangian_iteration_number = 0;
  T::timestepSetup();
}

template <>
void
AugmentedLagrangianContactProblemTempl<ReferenceResidualProblem>::addDefaultNonlinearConvergence(
    const InputParameters & params_to_apply)
{
  std::string class_name = "AugmentedLagrangianContactReferenceConvergence";
  InputParameters params = this->_factory.getValidParams(class_name);
  params.applyParameters(params_to_apply);
  params.applyParameters(parameters());
  params.set<bool>("added_as_default") = true;
  // TODO: Add multi-nonlinear system support
  if (this->numNonlinearSystems() > 1)
    mooseError("Multi-system not currently implemented");
  this->addConvergence(class_name, this->getNonlinearConvergenceNames()[0], params);
}

template <>
void
AugmentedLagrangianContactProblemTempl<FEProblem>::addDefaultNonlinearConvergence(
    const InputParameters & params_to_apply)
{
  std::string class_name = "AugmentedLagrangianContactFEProblemConvergence";
  InputParameters params = _factory.getValidParams(class_name);
  params.applyParameters(params_to_apply);
  params.applyParameters(parameters());
  params.set<bool>("added_as_default") = true;
  // TODO: Add multi-nonlinear system support
  if (this->numNonlinearSystems() > 1)
    mooseError("Multi-system not currently implemented");
  this->addConvergence(class_name, this->getNonlinearConvergenceNames()[0], params);
}

template class AugmentedLagrangianContactProblemTempl<ReferenceResidualProblem>;
template class AugmentedLagrangianContactProblemTempl<FEProblem>;
