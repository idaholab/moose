#include "ElasticEnergyMaterial.h"

template<>
InputParameters validParams<ElasticEnergyMaterial>()
{
  InputParameters params = validParams<DerivativeBaseMaterial>();
  params.addClassDescription("Free energy material for the elastic energy contributions.");
  params.addParam<std::string>("strain", "strain", "Name of the strain material property");
  return params;
}

ElasticEnergyMaterial::ElasticEnergyMaterial(const std::string & name,
                                             InputParameters parameters) :
    DerivativeBaseMaterial(name, parameters),
    _strain_name(getParam<std::string>("strain")),
    _strain(getMaterialProperty<>(_strain_name))
{
}
