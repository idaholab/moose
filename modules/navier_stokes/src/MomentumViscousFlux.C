#include "MomentumViscousFlux.h"
 

template<>
InputParameters validParams<MomentumViscousFlux>()
{
  InputParameters params;
  params.set<Real>("component") = -1;
  return params;
}

MomentumViscousFlux::MomentumViscousFlux(std::string name,
                  InputParameters parameters,
                  std::string var_name,
                  std::vector<std::string> coupled_to,
                  std::vector<std::string> coupled_as)
    :Kernel(name,parameters,var_name,true,coupled_to,coupled_as),
    _component(parameters.get<Real>("component"))
  {
    if(_component < 0)
    {
      std::cout<<"Must select a component for MomentumViscousFlux"<<std::endl;
      libmesh_error();
    }
  }

void
MomentumViscousFlux::subdomainSetup()
  {
    _viscous_stress_tensor = &_material->getTensorProperty("viscous_stress_tensor");
  }

Real
MomentumViscousFlux::computeQpResidual()
{
  RealTensorValue & vst = (*_viscous_stress_tensor)[_qp];

  RealVectorValue vec(vst(0,_component),vst(1,_component),vst(2,_component));

  return vec*_dtest[_i][_qp];
}
