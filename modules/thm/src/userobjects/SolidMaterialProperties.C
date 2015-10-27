#include "SolidMaterialProperties.h"
#include "R7Conversion.h"

template<>
InputParameters validParams<SolidMaterialProperties>()
{
  InputParameters params = validParams<GeneralUserObject>();

  params.addParam<std::string>("k", "Thermal conductivity");
  params.addParam<std::string>("Cp", "Specific heat");
  params.addParam<std::string>("rho", "Density");

  // These are here so we are able to control these values
  params.addPrivateParam<Real>("thermal_conductivity", 0.0);
  params.addPrivateParam<Real>("specific_heat", 0.0);
  params.addPrivateParam<Real>("density", 0.0);

  params.addPrivateParam<MultiMooseEnum>("execute_on");
  params.addPrivateParam<bool>("use_displaced_mesh");
  params.registerBase("SolidMaterialProperties");

  return params;
}

SolidMaterialProperties::SolidMaterialProperties(const InputParameters & parameters) :
    GeneralUserObject(parameters),
    ZeroInterface(parameters),
    _k_const(setConstRefParam(parameters, "k", "thermal_conductivity")),
    _Cp_const(setConstRefParam(parameters, "Cp", "specific_heat")),
    _rho_const(setConstRefParam(parameters, "rho", "density")),
    _k(_k_const==0 ? &getFunctionByName(getParam<std::string>("k")) : NULL),
    _Cp(_Cp_const==0 ? &getFunctionByName(getParam<std::string>("Cp")) : NULL),
    _rho(_rho_const==0 ? &getFunctionByName(getParam<std::string>("rho")) : NULL)
{
  mooseAssert(_k || _k_const != 0, "Thermal conductivity should never be zero");
  mooseAssert(_Cp || _Cp_const != 0, "Specific heat should never be zero");
  mooseAssert(_rho || _rho_const != 0, "Density should never be zero");
}

SolidMaterialProperties::~SolidMaterialProperties()
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
  if (_k != NULL)
    return _k->value(temp, Point());
  else
    return _k_const;
}

Real
SolidMaterialProperties::Cp(Real temp) const
{
  if (_Cp != NULL)
    return _Cp->value(temp, Point());
  else
    return _Cp_const;
}

Real
SolidMaterialProperties::rho(Real temp) const
{
  if (_rho != NULL)
    return _rho->value(temp, Point());
  else
    return _rho_const;
}

const Real &
SolidMaterialProperties::setConstRefParam(const InputParameters & params, std::string get_string, std::string set_string)
{
  std::string s = getParam<std::string>(get_string);
  if (isNumber(s))
  {
    return getParam<Real>(set_string);
  }
  else
  {
    // We won't be using whatever the value of _XYZ_const is, so set
    // the reference value to _real_zero
    return _real_zero;
  }
}
