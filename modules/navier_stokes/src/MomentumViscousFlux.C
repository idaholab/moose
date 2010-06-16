#include "MomentumViscousFlux.h"
 

template<>
InputParameters validParams<MomentumViscousFlux>()
{
  InputParameters params = validParams<Kernel>();
  params.set<Real>("component") = -1;
  return params;
}

MomentumViscousFlux::MomentumViscousFlux(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
   _component(parameters.get<Real>("component")),
   _viscous_stress_tensor(getTensorMaterialProperty("viscous_stress_tensor"))
{
  if(_component < 0)
  {
    std::cout<<"Must select a component for MomentumViscousFlux"<<std::endl;
    libmesh_error();
  }
}

Real
MomentumViscousFlux::computeQpResidual()
{
  RealTensorValue & vst = _viscous_stress_tensor[_qp];
  
  RealVectorValue vec(vst(0,_component),vst(1,_component),vst(2,_component));
  
  return vec*_grad_test[_i][_qp];
}
