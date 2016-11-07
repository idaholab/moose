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

#include "Damper.h"
#include "SystemBase.h"
#include "SubProblem.h"

template <>
InputParameters
validParams<Damper>()
{
  InputParameters params = validParams<MooseObject>();
  params.declareControllable("enable"); // allows Control to enable/disable this type of object
  params.registerBase("Damper");
  return params;
}

Damper::Damper(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    Restartable(parameters, "Dampers"),
    MeshChangedInterface(parameters),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _sys(*parameters.get<SystemBase *>("_sys"))
{
}
