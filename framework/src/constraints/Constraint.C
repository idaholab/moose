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
#include "Constraint.h"

#include "SystemBase.h"

template<>
InputParameters validParams<Constraint>()
{
  InputParameters params = validParams<MooseObject>();
  // Add the SetupInterface parameter, 'execute_on', default is 'residual'
  params += validParams<SetupInterface>();

  params.addRequiredParam<NonlinearVariableName>("variable", "The name of the variable that this constraint is applied to.");
  params.addParam<bool>("use_displaced_mesh", false, "Whether or not this object should use the displaced mesh for computation.  Note that in the case this is true but no displacements are provided in the Mesh block the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  params.registerBase("Constraint");

  return params;
}

Constraint::Constraint(const std::string & name, InputParameters parameters) :
  MooseObject(name, parameters),
  SetupInterface(parameters),
  FunctionInterface(parameters),
  UserObjectInterface(parameters),
  TransientInterface(parameters, name, "constraint"),
  GeometricSearchInterface(parameters),
  Restartable(name, parameters, "Constraints"),
  ZeroInterface(parameters),
  _subproblem(*parameters.get<SubProblem *>("_subproblem")),
  _sys(*parameters.get<SystemBase *>("_sys")),
  _tid(parameters.get<THREAD_ID>("_tid")),
  _assembly(_subproblem.assembly(_tid)),
  _var(_sys.getVariable(_tid, parameters.get<NonlinearVariableName>("variable"))),
  _mesh(_subproblem.mesh())
{
}

Constraint::~Constraint()
{
}
