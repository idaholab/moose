//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidProperties.h"

const std::string SolidProperties::_name = "";

template <>
InputParameters
validParams<SolidProperties>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.registerBase("SolidProperties");
  params.addClassDescription("Base class for defining solid property user objects");
  return params;
}

SolidProperties::SolidProperties(const InputParameters & parameters)
  : ThreadedGeneralUserObject(parameters)
{
}

const std::string &
SolidProperties::solidName() const
{
  return _name;
}

Real
SolidProperties::molarMass() const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " not implemented.");
}
