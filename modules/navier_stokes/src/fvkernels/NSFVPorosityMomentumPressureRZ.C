//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVPorosityMomentumPressureRZ.h"

#include "NS.h"

registerMooseObject("NavierStokesApp", NSFVPorosityMomentumPressureRZ);

InputParameters
NSFVPorosityMomentumPressureRZ::validParams()
{
  InputParameters params = NSFVMomentumPressureRZ::validParams();
  params.addClassDescription(
      "Adds the porous $-p/r$ term into the radial component of the Navier-Stokes "
      "momentum equation for the problems in the RZ coordinate system.");
  return params;
}

NSFVPorosityMomentumPressureRZ::NSFVPorosityMomentumPressureRZ(const InputParameters & params)
  : NSFVMomentumPressureRZ(params), _eps(getMaterialProperty<Real>(NS::porosity))
{
}

ADReal
NSFVPorosityMomentumPressureRZ::computeQpResidual()
{
  return _eps[_qp] * NSFVMomentumPressureRZ::computeQpResidual();
}
