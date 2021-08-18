#include "ADVectorVelocityComponentAux.h"

registerMooseObject("THMApp", ADVectorVelocityComponentAux);

InputParameters
ADVectorVelocityComponentAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("arhoA", "alpha*rho*A");
  params.addRequiredCoupledVar("arhouA", "alpha*rho*u*A");
  params.addRequiredParam<MaterialPropertyName>("direction",
                                                "Directional vector of 1D elements in 3D space");
  params.addRequiredParam<unsigned int>("component", "The vector component of interest");
  params.addClassDescription("Computes the component of a vector-valued velocity field given by "
                             "its magnitude and direction.");
  return params;
}

ADVectorVelocityComponentAux::ADVectorVelocityComponentAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _arhoA(coupledValue("arhoA")),
    _arhouA(coupledValue("arhouA")),
    _dir(getADMaterialProperty<RealVectorValue>("direction")),
    _component(getParam<unsigned int>("component"))
{
  if (isNodal())
    mooseError(name(), ": Does not support nodal variables.");
}

Real
ADVectorVelocityComponentAux::computeValue()
{
  return MetaPhysicL::raw_value(_dir[_qp])(_component) * _arhouA[_qp] / _arhoA[_qp];
}
