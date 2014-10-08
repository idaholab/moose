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
  params.registerBase("MaterialProperties");

  return params;
}

SolidMaterialProperties::SolidMaterialProperties(const std::string & name, InputParameters parameters) :
    GeneralUserObject(name, parameters),
    _k(NULL),
    _k_const(getParam<Real>("thermal_conductivity")),
    _Cp(NULL),
    _Cp_const(getParam<Real>("specific_heat")),
    _rho(NULL),
    _rho_const(getParam<Real>("density"))
{
  std::string s;

  s = getParam<std::string>("k");
  if (isNumber(s))
    this->parameters().set<Real>("thermal_conductivity") = toNumber(s);
  else
    _k = &getFunctionByName(s);

  s = getParam<std::string>("Cp");
  if (isNumber(s))
    this->parameters().set<Real>("specific_heat") = toNumber(s);
  else
    _Cp = &getFunctionByName(s);

  s = getParam<std::string>("rho");
  if (isNumber(s))
    this->parameters().set<Real>("density") = toNumber(s);
  else
    _rho = &getFunctionByName(s);
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
