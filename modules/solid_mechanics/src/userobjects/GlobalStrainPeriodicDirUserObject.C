//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GlobalStrainPeriodicDirUserObject.h"

registerMooseObject("SolidMechanicsApp", GlobalStrainPeriodicDirUserObject);

InputParameters
GlobalStrainPeriodicDirUserObject::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addClassDescription("Global Strain UserObject to retrieve periodic directions.");
  params.addRequiredCoupledVar("displacements", "The name of the displacement variables");
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;
  return params;
}

GlobalStrainPeriodicDirUserObject::GlobalStrainPeriodicDirUserObject(
    const InputParameters & parameters)
  : ElementUserObject(parameters),
    _dim(_mesh.dimension()),
    _disp_var(getFieldVars("displacements")),
    _periodic_dir()
{
  for (unsigned int dir = 0; dir < _dim; ++dir)
  {
    _periodic_dir(dir) = _mesh.isTranslatedPeriodic(*_disp_var[0], dir);

    for (unsigned int i = 1; i < _disp_var.size(); ++i)
      if (_mesh.isTranslatedPeriodic(*_disp_var[i], dir) != _periodic_dir(dir))
        mooseError("All the displacement components in a particular direction should have same "
                   "periodicity.");
  }
}

const VectorValue<bool> &
GlobalStrainPeriodicDirUserObject::getPeriodicDirections() const
{
  return _periodic_dir;
}

void
GlobalStrainPeriodicDirUserObject::initialize()
{
}

void
GlobalStrainPeriodicDirUserObject::execute()
{
}

void
GlobalStrainPeriodicDirUserObject::threadJoin(const UserObject & /* uo */)
{
}

void
GlobalStrainPeriodicDirUserObject::finalize()
{
}
