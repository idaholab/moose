//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Damper.h"
#include "SystemBase.h"
#include "SubProblem.h"
#include "Conversion.h"

InputParameters
Damper::validParams()
{
  InputParameters params = MooseObject::validParams();
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
    Restartable(this, "Dampers"),
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
    throw MooseException("From damper: '",
                         name(),
                         "' damping below min_damping: ",
                         cur_damping,
                         " Cutting timestep.");
}
