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

template <class T>
void
AugmentedLagrangianContactProblemTempl<T>::addDefaultConvergence()
{
  std::string class_name = "AugmentedLagrangianContactConvergence";
  InputParameters params = this->_factory.getValidParams(class_name);
  this->addConvergence(class_name, this->_nonlinear_convergence_name, params);
}

template class AugmentedLagrangianContactProblemTempl<ReferenceResidualProblem>;
template class AugmentedLagrangianContactProblemTempl<FEProblem>;
