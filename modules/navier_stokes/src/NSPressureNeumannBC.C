#include "NSPressureNeumannBC.h"

template<>
InputParameters validParams<NSPressureNeumannBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredCoupledVar("pressure", "");
  

  params.addRequiredParam<Real>("component", "(0,1,2) = (x,y,z) for which momentum component this BC is applied to");
  return params;
}

NSPressureNeumannBC::NSPressureNeumannBC(const std::string & name, InputParameters parameters)
  :IntegratedBC(name, parameters),
   _pressure(coupledValue("pressure")),
   _component(getParam<Real>("component")),
   _gamma(getMaterialProperty<Real>("gamma"))
{
}

Real
NSPressureNeumannBC::computeQpResidual()
{
  return _pressure[_qp] * _normals[_qp](_component) * _test[_i][_qp];
}
