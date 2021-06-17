//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CutSubdomainIDAux.h"

#include "XFEM.h"

registerMooseObject("XFEMApp", CutSubdomainIDAux);

InputParameters
CutSubdomainIDAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Fill the elemental variable with CutSubdomainID");
  params.addRequiredParam<UserObjectName>(
      "cut", "The geometric cut userobject that assigns the CutSubdomainID");
  return params;
}

CutSubdomainIDAux::CutSubdomainIDAux(const InputParameters & parameters)
  : AuxKernel(parameters), _cut(&getUserObject<GeometricCutUserObject>("cut"))
{
  if (isNodal())
    mooseError("CutSubdomainIDAux can only be run on an element variable");
}

Real
CutSubdomainIDAux::computeValue()
{
  return _cut->getCutSubdomainID(_current_elem);
}
