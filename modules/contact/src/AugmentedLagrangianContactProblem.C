/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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

// libMesh includes
#include "libmesh/string_to_enum.h"
#include "libmesh/sparse_matrix.h"

#include <limits>

template <>
InputParameters
validParams<AugmentedLagrangianContactProblem>()
{
  InputParameters params = validParams<ReferenceResidualProblem>();
  params.addParam<int>("maximum_lagrangian_update_iterations",
                       100,
                       "Maximum number of update Lagrangian Multiplier iterations per step");
  return params;
}

AugmentedLagrangianContactProblem::AugmentedLagrangianContactProblem(const InputParameters & params)
  : ReferenceResidualProblem(params),
    _num_lagmul_iterations(0),
    _max_lagmul_iters(getParam<int>("maximum_lagrangian_update_iterations"))
{
}

void
AugmentedLagrangianContactProblem::timestepSetup()
{
  _num_lagmul_iterations = 0;
  ReferenceResidualProblem::timestepSetup();
}

MooseNonlinearConvergenceReason
AugmentedLagrangianContactProblem::checkNonlinearConvergence(std::string & msg,
                                                             const PetscInt it,
                                                             const Real xnorm,
                                                             const Real snorm,
                                                             const Real fnorm,
                                                             const Real rtol,
                                                             const Real stol,
                                                             const Real abstol,
                                                             const PetscInt nfuncs,
                                                             const PetscInt /*max_funcs*/,
                                                             const Real ref_resid,
                                                             const Real /*div_threshold*/)
{

  Real my_max_funcs = std::numeric_limits<int>::max();
  Real my_div_threshold = std::numeric_limits<Real>::max();

  MooseNonlinearConvergenceReason reason =
      ReferenceResidualProblem::checkNonlinearConvergence(msg,
                                                          it,
                                                          xnorm,
                                                          snorm,
                                                          fnorm,
                                                          rtol,
                                                          stol,
                                                          abstol,
                                                          nfuncs,
                                                          my_max_funcs,
                                                          ref_resid,
                                                          my_div_threshold);

  _console << "Augmented Lagrangian contact iteration " << _num_lagmul_iterations << "\n";

  bool _augLM_repeat_step;

  if (reason > 0)
  {
    if (_num_lagmul_iterations < _max_lagmul_iters)
    {
      NonlinearSystemBase & nonlinear_sys = getNonlinearSystemBase();
      nonlinear_sys.update();

      const ConstraintWarehouse & constraints = nonlinear_sys.getConstraintWarehouse();

      std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> * penetration_locators =
          NULL;

      bool displaced = false;
      _augLM_repeat_step = false;
      if (getDisplacedProblem() == NULL)
      {
        GeometricSearchData & geom_search_data = geomSearchData();
        penetration_locators = &geom_search_data._penetration_locators;
      }
      else
      {
        GeometricSearchData & displaced_geom_search_data = getDisplacedProblem()->geomSearchData();
        penetration_locators = &displaced_geom_search_data._penetration_locators;
        displaced = true;
      }

      for (const auto & it : *penetration_locators)
      {
        PenetrationLocator & pen_loc = *(it.second);

        BoundaryID slave_boundary = pen_loc._slave_boundary;

        if (constraints.hasActiveNodeFaceConstraints(slave_boundary, displaced))
        {
          const auto & ncs = constraints.getActiveNodeFaceConstraints(slave_boundary, displaced);

          for (const auto & nc : ncs)
          {
            if (std::dynamic_pointer_cast<MechanicalContactConstraint>(nc) == NULL)
              mooseError("AugmentedLagrangianContactProblem: dynamic cast of "
                         "MechanicalContactConstraint object failed.");

            if (!(std::dynamic_pointer_cast<MechanicalContactConstraint>(nc))
                     ->AugmentedLagrangianContactConverged())
            {
              (std::dynamic_pointer_cast<MechanicalContactConstraint>(nc))
                  ->updateAugmentedLagrangianMultiplier(false);
              _augLM_repeat_step = true;
              break;
            }
          }
        }
      }

      // if (_augLM_repeat_step || _num_lagmul_iterations == 0)
      if (_augLM_repeat_step)
      { // force it to keep iterating
        reason = MOOSE_NONLINEAR_ITERATING;
        _console << "Augmented Lagrangian Multiplier needs updating\n " << std::endl;
        _num_lagmul_iterations++;
      }
      else
      {
        _console << "Augmented Lagrangian contact constraint enforcement is satisfied" << std::endl;
      }
    }
    else
    { // maxed out
      _console << "Maximum Augmented Lagrangian contact iterations have been reached" << std::endl;
      reason = MOOSE_DIVERGED_FUNCTION_COUNT;
    }
  }

  return reason;
}
