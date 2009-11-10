#include "PressureNeumannBC.h"

template<>
InputParameters valid_params<PressureNeumannBC>()
{
  InputParameters params;
  params.set<Real>("component");
  return params;
}

PressureNeumannBC::PressureNeumannBC(std::string name, InputParameters parameters, std::string var_name, unsigned int boundary_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as)
    :BoundaryCondition(name, parameters, var_name, true, boundary_id, coupled_to, coupled_as),
    _p(coupledValFace("p")),
    _pe(coupledValFace("pe")),
    _pu(coupledValFace("pu")),
    _pv(coupledValFace("pv")),
    _pw(_dim == 3 ? coupledValFace("pw") : _zero),
    _component(parameters.get<Real>("component"))
  {
    if(_component < 0)
    {
      std::cout<<"Must select a component for PressureNeumannBC"<<std::endl;
      libmesh_error();
    }
  }

Real
PressureNeumannBC::pressure()
  {
    //Only CONSTANT Real properties can be used by BCs
    Real gamma = _material->getConstantRealProperty("gamma");

    Real _u_vel = _pu[_qp] / _p[_qp];
    Real _v_vel = _pv[_qp] / _p[_qp];
    Real _w_vel = _pw[_qp] / _p[_qp];

    return (gamma - 1)*(_pe[_qp] - (0.5 * (_u_vel*_u_vel + _v_vel*_v_vel + _w_vel*_w_vel)));
  }

Real
PressureNeumannBC::computeQpResidual()
  {
    return pressure()*_normals_face[_qp](_component)*_phi_face[_i][_qp];
  }
