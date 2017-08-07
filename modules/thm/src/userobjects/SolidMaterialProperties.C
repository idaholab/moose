#include "SolidMaterialProperties.h"

template <>
InputParameters
validParams<SolidMaterialProperties>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addParam<FunctionName>("k", "Thermal conductivity");
  params.addParam<FunctionName>("Cp", "Specific heat");
  params.addParam<FunctionName>("rho", "Density");

  params.addPrivateParam<MultiMooseEnum>("execute_on");
  params.addPrivateParam<bool>("use_displaced_mesh");
  params.registerBase("SolidMaterialProperties");

  return params;
}

SolidMaterialProperties::SolidMaterialProperties(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    ZeroInterface(parameters),
    _k(getFunction("k")),
    _Cp(getFunction("Cp")),
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

Real
SolidMaterialProperties::k(Real temp) const
{
  return _k.value(temp, Point());
}

Real
SolidMaterialProperties::Cp(Real temp) const
{
  return _Cp.value(temp, Point());
}

Real
SolidMaterialProperties::rho(Real temp) const
{
  return _rho.value(temp, Point());
}
