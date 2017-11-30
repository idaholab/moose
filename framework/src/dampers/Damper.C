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
#include "Conversion.h"

template <>
InputParameters
validParams<Damper>()
{
  InputParameters params = validParams<MooseObject>();
  params.declareControllable("enable"); // allows Control to enable/disable this type of object
  params.registerBase("Damper");
  params.addParam<Real>("min_damping",
                        0.0,
                        "Minimum value of computed damping. Damping lower than "
                        "this will result in an exception being thrown and "
                        "cutting the time step");
  return params;
}

Damper::Damper(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    Restartable(parameters, "Dampers"),
    MeshChangedInterface(parameters),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _min_damping(getParam<Real>("min_damping"))
{
}

void
Damper::checkMinDamping(const Real cur_damping) const
{
  if (cur_damping < _min_damping)
    throw MooseException("From damper: '" + name() + "' damping below min_damping: " +
                         Moose::stringify(cur_damping) + " Cutting timestep.");
}
