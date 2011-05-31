#include "MomentumViscousFlux.h"
 

template<>
InputParameters validParams<MomentumViscousFlux>()
{
  // Initialize the params object from the base class
  InputParameters params = validParams<Kernel>();

  // Old style...
  // params.set<Real>("component") = -1;

  // component is a required parameter, so make it so!
  params.addRequiredParam<unsigned>("component", "");

  return params;
}




MomentumViscousFlux::MomentumViscousFlux(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _component(getParam<unsigned>("component")),
   _viscous_stress_tensor(getMaterialProperty<RealTensorValue>("viscous_stress_tensor"))
{
  if(_component > 2)
  {
    std::cout << "Must select a component<=2 for MomentumViscousFlux" << std::endl;
    libmesh_error();
  }
}




Real
MomentumViscousFlux::computeQpResidual()
{
  // Yay for less typing!
  RealTensorValue & vst = _viscous_stress_tensor[_qp];
  
  // _component'th column of vst...
  RealVectorValue vec(vst(0,_component),vst(1,_component),vst(2,_component));

  // ... dotted with grad(phi), note: sign is positive as this term was -div(tau) on the lhs
  return vec*_grad_test[_i][_qp];
}
