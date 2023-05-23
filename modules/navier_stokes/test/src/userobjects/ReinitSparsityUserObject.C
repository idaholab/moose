//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReinitSparsityUserObject.h"
#include "FEProblemBase.h"
#include "NonlinearSystemBase.h"
#include "libmesh/system.h"

registerMooseObject("NavierStokesTestApp", ReinitSparsityUserObject);

InputParameters
ReinitSparsityUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<Real>("reinit_time",
                                "At what time the nonlinear system matrix should be reinitialized");
  params.set<ExecFlagEnum>("execute_on") = {EXEC_TIMESTEP_BEGIN};
  params.suppressParameter<ExecFlagEnum>("execute_on");
  return params;
}

ReinitSparsityUserObject::ReinitSparsityUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters), _reinit_time(getParam<Real>("reinit_time"))
{
}

void
ReinitSparsityUserObject::execute()
{
  if (MooseUtils::relativeFuzzyEqual(_fe_problem.time(), _reinit_time))
    _fe_problem.getNonlinearSystemBase(0).system().reinit();
}
