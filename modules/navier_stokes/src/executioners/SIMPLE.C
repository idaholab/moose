//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "SIMPLE.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseApp.h"
#include "NonlinearSystem.h"
#include "KernelBase.h"
#include "INSFVMomentumPressure.h"

#include "libmesh/equation_systems.h"

registerMooseObject("NavierStokesApp", SIMPLE);

InputParameters
SIMPLE::validParams()
{
  InputParameters params = Executioner::validParams();
  params += FEProblemSolve::validParams();
  params.addRequiredParam<UserObjectName>("rhie_chow_user_object", "The rhie-chow user-object");
  params.addParam<NonlinearSystemName>(
      "momentum_system", "momentum_system", "The nonlinear system for the momentum equation");
  params.addParam<NonlinearSystemName>(
      "pressure_system", "pressure_system", "The nonlinear system for the pressure equation");
  params.addParam<std::string>("momentum_tag",
                               "non_pressure_momentum_kernels",
                               "The name of the tags associated with the kernels in the momentum "
                               "equations which are not related to the pressure gradient.");
  return params;
}

SIMPLE::SIMPLE(const InputParameters & parameters)
  : Executioner(parameters),
    _problem(_fe_problem),
    _feproblem_solve(*this),
    _time_step(_problem.timeStep()),
    _time(_problem.time()),
    _momentum_sys_number(_problem.nlSysNum(getParam<NonlinearSystemName>("momentum_system"))),
    _pressure_sys_number(_problem.nlSysNum(getParam<NonlinearSystemName>("pressure_system"))),
    _momentum_sys(_problem.getNonlinearSystemBase(_momentum_sys_number)),
    _pressure_sys(_problem.getNonlinearSystemBase(_pressure_sys_number)),
    _momentum_tag_name(getParam<std::string>("momentum_tag")),
    _momentum_tag_id(_problem.addVectorTag(_momentum_tag_name))
{
  _fixed_point_solve->setInnerSolve(_feproblem_solve);

  _momentum_sys.addVector(_momentum_tag_id, false, ParallelType::PARALLEL);

  _time = 0;
}

void
SIMPLE::init()
{
  _problem.initialSetup();
  _rc_uo = const_cast<INSFVRhieChowInterpolatorSegregated *>(
      &getUserObject<INSFVRhieChowInterpolatorSegregated>("rhie_chow_user_object"));
  _rc_uo->linkMomentumSystem(_momentum_sys, _momentum_tag_id);
}

void
SIMPLE::execute()
{
  _problem.timestepSetup();

  preSolve();
  _problem.execute(EXEC_TIMESTEP_BEGIN);
  _problem.outputStep(EXEC_TIMESTEP_BEGIN);
  _problem.updateActiveObjects();

  _problem.solve(_momentum_sys_number);

  _rc_uo->execute();

  _console << "This will be something smart" << std::endl;
}
