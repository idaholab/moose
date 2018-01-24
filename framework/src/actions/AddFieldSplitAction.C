/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE includes
#include "AddFieldSplitAction.h"
#include "FEProblem.h"
#include "NonlinearSystemBase.h"

template <>
InputParameters
validParams<AddFieldSplitAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  params.addParam<std::string>("type", "Split", "Classname of the split object");
  params.addParam<std::vector<NonlinearVariableName>>("vars", "variables for this field");
  params.addParam<MultiMooseEnum>(
      "petsc_options", Moose::PetscSupport::getCommonPetscFlags(), "Singleton PETSc options");
  params.addParam<MultiMooseEnum>("petsc_options_iname",
                                  Moose::PetscSupport::getCommonPetscKeys(),
                                  "Names of PETSc name/value pairs");
  params.addParam<std::vector<std::string>>(
      "petsc_options_value",
      "Values of PETSc name/value pairs (must correspond with \"petsc_options_iname\"");
  return params;
}

AddFieldSplitAction::AddFieldSplitAction(InputParameters params) : MooseObjectAction(params) {}

void
AddFieldSplitAction::act()
{
  _moose_object_pars.set<FEProblemBase *>("_fe_problem_base") = _problem.get();
  _problem->getNonlinearSystemBase().addSplit(_type, _name, _moose_object_pars);
}
