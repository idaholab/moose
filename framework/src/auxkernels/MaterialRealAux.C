
#include "MaterialRealAux.h"

template<>
InputParameters validParams<MaterialRealAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<std::string>("matpro", "The material parameter name.");
  return params;
}

MaterialRealAux::MaterialRealAux(const std::string & name, InputParameters parameters)
  :AuxKernel(name, parameters),
  _matpro(getParam<std::string>("matpro")),
  _prop(getMaterialProperty<Real>(_matpro))
{}


Real
MaterialRealAux::computeValue()
{
 return _prop[_qp];
}
