#include "SiHeatConductionMaterial.h"

template<>
InputParameters validParams<SiHeatConductionMaterial>()
{
  InputParameters params = validParams<HeatConductionMaterial>();
  return params;
}

SiHeatConductionMaterial::SiHeatConductionMaterial(const std::string & name, InputParameters parameters) :
    HeatConductionMaterial(name, parameters)
{}

void
SiHeatConductionMaterial::computeProperties()
{
  HeatConductionMaterial::computeProperties();
  
  for(unsigned int qp=0; qp<_qrule->n_points(); ++qp)
  {
    _thermal_conductivity[qp] = 615.3 * std::exp(-0.006284 * _temp[qp]) +
      + 64.19 * std::exp(-0.0006563 * _temp[qp]) * 1e-6;
  }
}
