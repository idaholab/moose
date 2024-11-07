//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AugmentedLagrangianContactConvergence.h"

// MOOSE includes
#include "AuxiliarySystem.h"
#include "DisplacedProblem.h"
#include "MooseApp.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "NearestNodeLocator.h"
#include "NonlinearSystem.h"
#include "PenetrationLocator.h"
#include "FEProblemBase.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "Executioner.h"
#include "AddVariableAction.h"
#include "ConstraintWarehouse.h"
#include "MortarUserObject.h"
#include "AugmentedLagrangeInterface.h"
#include "DefaultNonlinearConvergence.h"
#include "ReferenceResidualConvergence.h"
#include "MechanicalContactConstraint.h"

registerMooseObject("ContactApp", AugmentedLagrangianContactReferenceConvergence);
registerMooseObject("ContactApp", AugmentedLagrangianContactFEProblemConvergence);

template <class T>
InputParameters
AugmentedLagrangianContactConvergence<T>::validParams()
{
  InputParameters params = T::validParams();
  params += AugmentedLagrangianContactProblemInterface::validParams();
  params.addClassDescription("Convergence for augmented Lagrangian contact");
  return params;
}

template <class T>
AugmentedLagrangianContactConvergence<T>::AugmentedLagrangianContactConvergence(
    const InputParameters & params)
  : T(params), AugmentedLagrangianContactProblemInterface(params)
{
}

template <class T>
Convergence::MooseConvergenceStatus
AugmentedLagrangianContactConvergence<T>::checkConvergence(unsigned int iter)
{
  // Check convergence of the nonlinear problem
  auto reason = T::checkConvergence(iter);

  auto & fe_problem_base = this->getMooseApp().feProblem();

  auto aug_contact = dynamic_cast<AugmentedLagrangianContactProblemInterface *>(&fe_problem_base);
  _lagrangian_iteration_number = aug_contact->getLagrangianIterationNumber();

  bool repeat_augmented_lagrange_step = false;

  // Nonlinear solve is converged, now check that the constraints are met
  if (reason == Convergence::MooseConvergenceStatus::CONVERGED)
  {
    if (_lagrangian_iteration_number < _maximum_number_lagrangian_iterations)
    {

      auto & nonlinear_sys = fe_problem_base.currentNonlinearSystem();
      nonlinear_sys.update();

      // Get the penetration locator from the displaced mesh if it exist, otherwise get
      // it from the undisplaced mesh.
      const auto displaced_problem = fe_problem_base.getDisplacedProblem();
      const auto & penetration_locators = (displaced_problem ? displaced_problem->geomSearchData()
                                                             : fe_problem_base.geomSearchData())
                                              ._penetration_locators;

      // loop over contact pairs (penetration locators)
      const ConstraintWarehouse & constraints = nonlinear_sys.getConstraintWarehouse();
      std::list<std::shared_ptr<MechanicalContactConstraint>> mccs;
      for (const auto & pair : penetration_locators)
      {
        const auto & boundaries = pair.first;

        if (!constraints.hasActiveNodeFaceConstraints(boundaries.second, bool(displaced_problem)))
          continue;
        const auto & ncs =
            constraints.getActiveNodeFaceConstraints(boundaries.second, bool(displaced_problem));

        mccs.emplace_back(nullptr);
        for (const auto & nc : ncs)
          if (const auto mcc = std::dynamic_pointer_cast<MechanicalContactConstraint>(nc); !mcc)
            mooseError("AugmentedLagrangianContactProblem: dynamic cast of "
                       "MechanicalContactConstraint object failed.");
          else
          {
            // Return if this constraint does not correspond to the primary-secondary pair
            // prepared by the outer loops.
            // This continue statement is required when, e.g. one secondary surface constrains
            // more than one primary surface.
            if (mcc->secondaryBoundary() != boundaries.second ||
                mcc->primaryBoundary() != boundaries.first)
              continue;

            // save one constraint pointer for each contact pair
            if (!mccs.back())
              mccs.back() = mcc;

            // check if any of the constraints is not yet converged
            if (repeat_augmented_lagrange_step || !mcc->AugmentedLagrangianContactConverged())
            {
              repeat_augmented_lagrange_step = true;
              break;
            }
          }
      }

      // next loop over penalty mortar user objects
      const auto & pmuos = this->_app.template getInterfaceObjects<AugmentedLagrangeInterface>();
      for (auto * pmuo : pmuos)
      {
        // check if any of the constraints is not yet converged
        if (!repeat_augmented_lagrange_step && !pmuo->isAugmentedLagrangianConverged())
          repeat_augmented_lagrange_step = true;
      }

      // Communicate the repeat_augmented_lagrange_step in parallel.
      // If one proc needs to do another loop, all do.
      this->_communicator.max(repeat_augmented_lagrange_step);

      // repeat update step if necessary
      if (repeat_augmented_lagrange_step)
      {
        _lagrangian_iteration_number++;
        Moose::out << "Augmented Lagrangian contact repeat " << _lagrangian_iteration_number
                   << '\n';

        // Each contact pair will have constraints for all displacements, but those share the
        // Lagrange multipliers, which are stored on the penetration locator. We call update
        // only for the first constraint for each contact pair.
        for (const auto & mcc : mccs)
          mcc->updateAugmentedLagrangianMultiplier(/* beginning_of_step = */ false);

        // Update all penalty mortar user objects
        for (const auto & pmuo : pmuos)
          pmuo->updateAugmentedLagrangianMultipliers();

        // call AM setup again (e.g. to update active sets)
        for (const auto & pmuo : pmuos)
          pmuo->augmentedLagrangianSetup();

        // force it to keep iterating
        reason = Convergence::MooseConvergenceStatus::ITERATING;
        Moose::out << "Augmented Lagrangian Multiplier needs updating.";
      }
      else
        Moose::out << "Augmented Lagrangian contact constraint enforcement is satisfied.";
    }
    else
    {
      // maxed out
      Moose::out << "Maximum Augmented Lagrangian contact iterations have been reached.";
      reason = Convergence::MooseConvergenceStatus::DIVERGED;
    }
  }

  return reason;
}

template class AugmentedLagrangianContactConvergence<ReferenceResidualConvergence>;
template class AugmentedLagrangianContactConvergence<DefaultNonlinearConvergence>;
