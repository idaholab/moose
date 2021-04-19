//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CutElementSubdomainModifier.h"

registerMooseObject("XFEMApp", CutElementSubdomainModifier);

InputParameters
CutElementSubdomainModifier::validParams()
{
  InputParameters params = ElementSubdomainModifier::validParams();
  params.addClassDescription("Change element subdomain based on CutSubdomainID");
  params.addRequiredParam<UserObjectName>(
      "geometric_cut_userobject", "The geometric cut userobject that assigns the CutSubdomainID");
  return params;
}

CutElementSubdomainModifier::CutElementSubdomainModifier(const InputParameters & parameters)
  : ElementSubdomainModifier(parameters),
    _cut(&getUserObject<GeometricCutUserObject>("geometric_cut_userobject"))
{
}

SubdomainID
CutElementSubdomainModifier::computeSubdomainID()
{
  return _cut->getCutSubdomainID(_current_elem);
}
