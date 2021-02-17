//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeometricCutSubdomainIDAux.h"

#include "XFEM.h"

registerMooseObject("XFEMApp", GeometricCutSubdomainIDAux);

InputParameters
GeometricCutSubdomainIDAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Fill the elemental variable with GeometricCutSubdomainID");
  params.addRequiredParam<UserObjectName>(
      "cut", "The geometric cut userobject that assigns the GeometricCutSubdomainID");
  return params;
}

GeometricCutSubdomainIDAux::GeometricCutSubdomainIDAux(const InputParameters & parameters)
  : AuxKernel(parameters), _cut(&getUserObject<GeometricCutUserObject>("cut"))
{
  if (isNodal())
    mooseError("GeometricCutSubdomainIDAux can only be run on an element variable");
}

Real
GeometricCutSubdomainIDAux::computeValue()
{
  return _cut->getGeometricCutSubdomainID(_current_elem);
}
