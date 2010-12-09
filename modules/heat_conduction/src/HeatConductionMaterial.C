#include "HeatConductionMaterial.h"

template<>
InputParameters validParams<HeatConductionMaterial>()
{
  InputParameters params = validParams<Material>();

  params.addCoupledVar("temp", "Coupled Temperature");
  
  params.set<Real>("thermal_conductivity")=1.0;
  params.set<Real>("thermal_expansion")=1.0;
  params.set<Real>("specific_heat")=1.0;
  params.set<Real>("density")=1.0;
  params.set<Real>("youngs_modulus")=1.0;
  params.set<Real>("poissons_ratio")=1.0;
  params.set<Real>("neutron_diffusion_coefficient")=1.0;
  params.set<Real>("neutron_absorption_xs")=1.0;
  params.set<Real>("neutron_fission_xs")=1.0;
  params.set<Real>("neutron_per_fission")=1.0;
  params.set<Real>("neutron_velocity")=1.0;
  params.set<Real>("t_ref")=300;
  params.set<Real>("d_1")=1.0;
  params.set<Real>("d_2")=0.0;
  params.set<Real>("q_1")=1.0;
  params.set<Real>("q_2")=0.0;
  params.set<Real>("gas_constant")=8.3143e-3;
  params.set<Real>("lambda")=1.;

  return params;
}

HeatConductionMaterial::HeatConductionMaterial(const std::string & name,
                   InputParameters parameters)
  :Material(name, parameters),

   _has_temp(isCoupled("temp")),
   _temp(_has_temp ? coupledValue("temp") : _zero),

   _my_thermal_conductivity(getParam<Real>("thermal_conductivity")),
   _my_specific_heat(getParam<Real>("specific_heat")),
   _my_density(getParam<Real>("density")),

   _thermal_conductivity(declareProperty<Real>("thermal_conductivity")),
   _specific_heat(declareProperty<Real>("specific_heat")),
   _density(declareProperty<Real>("density"))
  {}

void
HeatConductionMaterial::computeProperties()
{
  for(unsigned int qp=0; qp<_n_qpoints; qp++)
  {
    _density[qp] = _my_density;
    _thermal_conductivity[qp] = _my_thermal_conductivity;
    _specific_heat[qp] = _my_specific_heat;
  }
}
