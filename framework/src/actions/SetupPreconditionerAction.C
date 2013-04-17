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

#include "SetupPreconditionerAction.h"
#include "Factory.h"
#include "PetscSupport.h"
#include "MoosePreconditioner.h"
#include "FEProblem.h"

unsigned int SetupPreconditionerAction::_count = 0;

template<>
InputParameters validParams<SetupPreconditionerAction>()
{
  InputParameters params = validParams<MooseObjectAction>();

#ifdef LIBMESH_HAVE_PETSC
  MooseEnum common_petsc_options("-ksp_monitor, -snes_mf_operator", "", true);
  std::vector<MooseEnum> common_petsc_options_vec(1, common_petsc_options);

  params.addParam<std::vector<MooseEnum> >("petsc_options", common_petsc_options_vec, "Singleton Petsc options");
  params.addParam<std::vector<std::string> >("petsc_options_iname", "Names of Petsc name/value pairs");
  params.addParam<std::vector<std::string> >("petsc_options_value", "Values of Petsc name/value pairs (must correspond with \"petsc_options_iname\"");
#endif //LIBMESH_HAVE_PETSC

  return params;
}

SetupPreconditionerAction::SetupPreconditionerAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
}

void
SetupPreconditionerAction::act()
{
  if (_problem != NULL)
  {
    // build the preconditioner
    _moose_object_pars.set<FEProblem *>("_fe_problem") = _problem;
    MoosePreconditioner * pc = dynamic_cast<MoosePreconditioner *>(_factory.create(_type, getShortName(), _moose_object_pars));
    if (pc == NULL)
      mooseError("Failed to build the preconditioner.");

    _problem->getNonlinearSystem().setPreconditioner(pc);

    /**
     * Go ahead and set common precondition options here.  The child classes will still be called
     * through the action warehouse
     */
#ifdef LIBMESH_HAVE_PETSC
    _problem->storePetscOptions(getParam<std::vector<MooseEnum> >("petsc_options"),
                                getParam<std::vector<std::string> >("petsc_options_iname"), getParam<std::vector<std::string> >("petsc_options_value"));
    Moose::PetscSupport::petscSetOptions(*_problem);
#endif //LIBMESH_HAVE_PETSC
  }
}
