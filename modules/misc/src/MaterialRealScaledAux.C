
#include "MaterialRealScaledAux.h"

template<>
InputParameters validParams<MaterialRealScaledAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<std::string>("matpro", "The material parameter name.");
  params.addParam<Real>("factor", 1, "The factor by which to multiply your material property for visualization");
  params.addParam<Real>("offset", 0, "The offset to add to your material property for visualization");
  return params;
}

MaterialRealScaledAux::MaterialRealScaledAux(const std::string & name, InputParameters parameters)
    :AuxKernel(name, parameters),
     _matpro(getParam<std::string>("matpro")),
     _prop(getMaterialProperty<Real>(_matpro)),
     _factor(getParam<Real>("factor")),
     _offset(getParam<Real>("offset"))
{}

Real
MaterialRealScaledAux::computeValue()
{
  return _factor
    * _prop[_qp]
    + _offset;
}
