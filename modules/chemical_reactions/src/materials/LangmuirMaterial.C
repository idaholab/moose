#include "LangmuirMaterial.h"


template<>
InputParameters validParams<LangmuirMaterial>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredParam<Real>("mat_desorption_time_const", "Time constant for Langmuir desorption (gas moving from matrix to porespace).  If the total desorption time constant is nonpositive then no desorption takes place (so set this negative if you don't want desorption).  Units [s]");
  params.addRequiredParam<Real>("mat_adsorption_time_const", "Time constant for Langmuir adsorption (gas moving from porespace to matrix).  Units [s].  If the total adsorption time constant is nonpositive no adsorption takes place (so set this negative if you don't want adsorption)");
  params.addCoupledVar("desorption_time_const_change", "An auxillary variable describing time_const changes.  time_const = mat_time_const + time_const_change.  If this is not provided, zero is used.  If the total time_const is nonpositive, no desorption takes place.");
  params.addCoupledVar("adsorption_time_const_change", "An auxillary variable describing time_const changes.  time_const = mat_time_const + time_const_change.  If this is not provided, zero is used.  If the total time_const is nonpositive then no adsorption takes place");
  params.addRequiredParam<Real>("mat_langmuir_density", "This is (Langmuir volume)*(density of gas at standard temp and pressure).  Langmuir volume is measured in (gas volume)/(matrix volume).  (Methane density(101kPa, 20degC) = 0.655kg/m^3.  Methane density(101kPa, 0degC) = 0.715kg/m^3.)  Units [kg/m^3]");
  params.addRequiredParam<Real>("mat_langmuir_pressure", "Langmuir pressure.  Units Pa");
  params.addRequiredCoupledVar("pressure_var", "The variable that is used to calculate equilibrium concentration");
  params.addClassDescription("Material type that holds info regarding Langmuir desorption from matrix to porespace and viceversa");
  return params;
}

LangmuirMaterial::LangmuirMaterial(const std::string & name,
                                 InputParameters parameters) :
    Material(name, parameters),
    _mat_desorption_time_const(getParam<Real>("mat_desorption_time_const")),
    _mat_adsorption_time_const(getParam<Real>("mat_adsorption_time_const")),
    _desorption_time_const_change(isCoupled("desorption_time_const_change") ? &coupledValue("desorption_time_const_change") : &_zero), // coupledValue returns a reference (an alias) to a VariableValue, and the & turns it into a pointer
    _adsorption_time_const_change(isCoupled("adsorption_time_const_change") ? &coupledValue("adsorption_time_const_change") : &_zero), // coupledValue returns a reference (an alias) to a VariableValue, and the & turns it into a pointer
    _mat_langmuir_density(getParam<Real>("mat_langmuir_density")),
    _mat_langmuir_pressure(getParam<Real>("mat_langmuir_pressure")),

    _pressure(&coupledValue("pressure_var")),

    _desorption_time_const(declareProperty<Real>("desorption_time_const")),
    _adsorption_time_const(declareProperty<Real>("adsorption_time_const")),
    _equilib_conc(declareProperty<Real>("desorption_equilib_conc")),
    _equilib_conc_prime(declareProperty<Real>("desorption_equilib_conc_prime"))
{}

void
LangmuirMaterial::computeQpProperties()
{
  _desorption_time_const[_qp] = _mat_desorption_time_const + (*_desorption_time_const_change)[_qp];
  _adsorption_time_const[_qp] = _mat_adsorption_time_const + (*_adsorption_time_const_change)[_qp];
  _equilib_conc[_qp] = _mat_langmuir_density*((*_pressure)[_qp])/(_mat_langmuir_pressure + (*_pressure)[_qp]);
  _equilib_conc_prime[_qp] = _mat_langmuir_density/(_mat_langmuir_pressure + (*_pressure)[_qp]) - _mat_langmuir_density*((*_pressure)[_qp])/std::pow((_mat_langmuir_pressure + (*_pressure)[_qp]), 2);
}
