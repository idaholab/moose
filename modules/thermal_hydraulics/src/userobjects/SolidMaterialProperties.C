//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidMaterialProperties.h"

registerMooseObject("ThermalHydraulicsApp", SolidMaterialProperties);

InputParameters
SolidMaterialProperties::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<FunctionName>("k", "Thermal conductivity [W/(m-K)]");
  params.addDeprecatedParam<FunctionName>(
      "Cp",
      "Specific heat [J/(kg-K)]",
      "The parameter 'Cp' has been deprecated. Use 'cp' instead.");
  params.addParam<FunctionName>("cp", "Specific heat [J/(kg-K)]");
  params.addRequiredParam<FunctionName>("rho", "Density [kg/m^3]");

  params.addPrivateParam<ExecFlagEnum>("execute_on");
  params.addPrivateParam<bool>("use_displaced_mesh");

  params.declareControllable("rho cp k");

  params.registerBase("SolidMaterialProperties");

  return params;
}

SolidMaterialProperties::SolidMaterialProperties(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _k(getFunction("k")),
    _cp(isParamValid("Cp") ? getFunction("Cp") : getFunction("cp")),
    _rho(getFunction("rho"))
{
}

void
SolidMaterialProperties::initialize()
{
}

void
SolidMaterialProperties::execute()
{
}

void
SolidMaterialProperties::finalize()
{
}

ADReal
SolidMaterialProperties::k(const ADReal & temp) const
{
  return _k.value(temp, ADPoint());
}

ADReal
SolidMaterialProperties::cp(const ADReal & temp) const
{
  return _cp.value(temp, ADPoint());
}

ADReal
SolidMaterialProperties::rho(const ADReal & temp) const
{
  return _rho.value(temp, ADPoint());
}
