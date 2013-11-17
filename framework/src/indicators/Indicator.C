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

#include "Indicator.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"

// libmesh includes
#include "libmesh/threads.h"

template<>
InputParameters validParams<Indicator>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<BlockRestrictable>();

  params.addParam<bool>("use_displaced_mesh", false, "Whether or not this object should use the displaced mesh for computation.  Note that in the case this is true but no displacements are provided in the Mesh block the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");
  params.addPrivateParam<std::string>("built_by_action", "add_indicator");
  return params;
}


Indicator::Indicator(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    BlockRestrictable(name, parameters),
    SetupInterface(parameters),
    FunctionInterface(parameters),
    UserObjectInterface(parameters),
    Restartable(name, parameters, "Indicators"),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _fe_problem(*parameters.get<FEProblem *>("_fe_problem")),
    _sys(*parameters.get<SystemBase *>("_sys")),
    _solution(_sys.solution()),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),

    _mesh(_subproblem.mesh())
//    _dim(_mesh.dimension())
{
}

void
Indicator::IndicatorSetup()
{
}
