//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MechanicalStretchPD.h"

registerMooseObject("PeridynamicsApp", MechanicalStretchPD);

template <>
InputParameters
validParams<MechanicalStretchPD>()
{
  InputParameters params = validParams<AuxKernelBasePD>();
  params.addClassDescription("Class for outputing bond mechanical stretch value");
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;

  return params;
}

MechanicalStretchPD::MechanicalStretchPD(const InputParameters & parameters)
  : AuxKernelBasePD(parameters),
    _mechanical_stretch(getMaterialProperty<Real>("mechanical_stretch"))
{
}

Real
MechanicalStretchPD::computeValue()
{
  return _mechanical_stretch[0];
}
