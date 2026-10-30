//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "Predictor.h"

#include "SystemBase.h"
#include "Assembly.h"
#include "Executioner.h"
#include "AddVariableAction.h"
#include "ConstraintWarehouse.h"
#include "MortarUserObject.h"
#include "AugmentedLagrangeInterface.h"
#include "DefaultNonlinearConvergence.h"

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
  : T(params),
    AugmentedLagrangianContactProblemInterface(params),
    _augmented_lagrangian_failed(false),
    _n_nonlinear_iterations(0),
    _n_linear_iterations(0)
{
  // TODO: Add multi-nonlinear system support
  if (this->numNonlinearSystems() > 1)
    mooseError("Multi-system not currently implemented");
}

template <class T>
void
AugmentedLagrangianContactProblemTempl<T>::solve(const unsigned int nl_sys_num)
{
  _lagrangian_iteration_number = 0;
  _augmented_lagrangian_failed = false;
  _n_nonlinear_iterations = 0;
  _n_linear_iterations = 0;

  auto & nl = this->getNonlinearSystemBase(nl_sys_num);

  // The solves after a multiplier update start from the converged solution of the previous
  // solve, so the predictor and a per-step scaling computation only apply to the first one
  Predictor * const predictor = nl.getPredictor();
  const auto predictor_ptr =
      predictor ? std::static_pointer_cast<Predictor>(predictor->getSharedPtr()) : nullptr;
  const bool compute_scaling_once = nl.computeScalingOnce();

  // A multiplier update changes the residual, but the default nonlinear convergence test (through
  // nl_abs_tol) and a reference residual test can accept the first residual of a solve without a
  // Newton step. The solution would then not respond to the new multipliers. At least one Newton
  // step after each update prevents that.
  auto & convergence = cast_ref<DefaultNonlinearConvergence &>(
      this->getConvergence(this->getNonlinearConvergenceNames()[nl_sys_num]));
  const auto forced_its = convergence.getForcedIterations();

  const auto restore = [&]()
  {
    nl.setPredictor(predictor_ptr);
    nl.computeScalingOnce(compute_scaling_once);
    convergence.setForcedIterations(forced_its);
  };

  try
  {
    while (true)
    {
      T::solve(nl_sys_num);
      _n_nonlinear_iterations += nl.nNonlinearIterations();
      _n_linear_iterations += nl.nLinearIterations();

      if (!this->shouldSolve() || !T::solverSystemConverged(nl_sys_num))
        break;

      if (updateAugmentedLagrangianMultipliers() || _augmented_lagrangian_failed)
        break;

      if (_lagrangian_iteration_number == 1)
      {
        nl.setPredictor(nullptr);
        nl.computeScalingOnce(true);
        convergence.setForcedIterations(std::max(forced_its, 1u));
      }
    }
  }
  catch (...)
  {
    restore();
    throw;
  }

  restore();
}

template <class T>
bool
AugmentedLagrangianContactProblemTempl<T>::updateAugmentedLagrangianMultipliers()
{
  bool repeat_augmented_lagrange_step = false;

  // Get the penetration locator from the displaced mesh if it exist, otherwise get
  // it from the undisplaced mesh.
  const auto displaced_problem = this->getDisplacedProblem();
  const auto & penetration_locators =
      (displaced_problem ? displaced_problem->geomSearchData() : this->geomSearchData())
          ._penetration_locators;

  // loop over contact pairs (penetration locators)
  const ConstraintWarehouse & constraints = this->currentNonlinearSystem().getConstraintWarehouse();
  std::list<std::shared_ptr<MechanicalContactConstraint>> mccs;
  for (const auto & [boundaries, _] : penetration_locators)
  {
    const auto & [primary_id, secondary_id] = boundaries;

    if (!constraints.hasActiveNodeFaceConstraints(secondary_id, bool(displaced_problem)))
      continue;
    const auto & ncs =
        constraints.getActiveNodeFaceConstraints(secondary_id, bool(displaced_problem));

    std::shared_ptr<MechanicalContactConstraint> first_mcc;
    for (const auto & nc : ncs)
    {
      // Other node-face constraints, and contact constraints that do not use the augmented
      // Lagrangian formulation, take no part in the augmented Lagrangian iteration
      const auto mcc = std::dynamic_pointer_cast<MechanicalContactConstraint>(nc);
      if (!mcc || !mcc->isAugmentedLagrangian())
        continue;

      // Return if this constraint does not correspond to the primary-secondary pair
      // prepared by the outer loops.
      // This continue statement is required when, e.g. one secondary surface constrains
      // more than one primary surface.
      if (mcc->secondaryBoundary() != secondary_id || mcc->primaryBoundary() != primary_id)
        continue;

      // save one constraint pointer for each contact pair
      if (!first_mcc)
        first_mcc = mcc;

      // check if any of the constraints is not yet converged. The check is collective, so
      // it must run for every constraint on every rank.
      if (!mcc->AugmentedLagrangianContactConverged())
        repeat_augmented_lagrange_step = true;
    }

    if (first_mcc)
      mccs.push_back(first_mcc);
  }

  // next loop over penalty mortar user objects
  const auto & pmuos = this->_app.template getInterfaceObjects<AugmentedLagrangeInterface>();
  for (auto * pmuo : pmuos)
  {
    // check if any of the constraints is not yet converged. The check is collective, so it
    // must run for every user object on every rank.
    if (!pmuo->isAugmentedLagrangianConverged())
      repeat_augmented_lagrange_step = true;
  }

  // Communicate the repeat_augmented_lagrange_step in parallel.
  // If one proc needs to do another loop, all do.
  this->_communicator.max(repeat_augmented_lagrange_step);

  if (!repeat_augmented_lagrange_step)
  {
    this->_console << "Augmented Lagrangian contact constraint enforcement is satisfied."
                   << std::endl;
    return true;
  }

  if (_lagrangian_iteration_number >= _maximum_number_lagrangian_iterations)
  {
    this->_console << "Maximum Augmented Lagrangian contact iterations have been reached."
                   << std::endl;
    _augmented_lagrangian_failed = true;
    return false;
  }

  _lagrangian_iteration_number++;
  this->_console << "Augmented Lagrangian contact repeat " << _lagrangian_iteration_number
                 << std::endl;

  // Each contact pair will have constraints for all displacements, but those share the
  // Lagrange multipliers, which are stored on the penetration locator. We call update
  // only for the first constraint for each contact pair.
  for (const auto & mcc : mccs)
    mcc->updateAugmentedLagrangianMultiplier(/* beginning_of_step = */ false);

  // Update all penalty mortar user objects
  for (const auto & pmuo : pmuos)
    pmuo->updateAugmentedLagrangianMultipliers();

  // Store the current gaps, which the next multiplier update compares against to adapt the penalty
  for (const auto & pmuo : pmuos)
    pmuo->augmentedLagrangianSetup();

  return false;
}

template <class T>
bool
AugmentedLagrangianContactProblemTempl<T>::solverSystemConverged(const unsigned int sys_num)
{
  return !_augmented_lagrangian_failed && T::solverSystemConverged(sys_num);
}

template <class T>
unsigned int
AugmentedLagrangianContactProblemTempl<T>::nNonlinearIterations(const unsigned int) const
{
  return _n_nonlinear_iterations;
}

template <class T>
unsigned int
AugmentedLagrangianContactProblemTempl<T>::nLinearIterations(const unsigned int) const
{
  return _n_linear_iterations;
}

template class AugmentedLagrangianContactProblemTempl<ReferenceResidualProblem>;
template class AugmentedLagrangianContactProblemTempl<FEProblem>;
