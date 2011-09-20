#include "NSMomentumWeakStagnationBC.h"

template<>
InputParameters validParams<NSMomentumWeakStagnationBC>()
{
  InputParameters params = validParams<NSWeakStagnationBC>();

  // Required parameters
  params.addRequiredParam<unsigned>("component", "(0,1,2) = (x,y,z) for which momentum component this BC is applied to");

  return params;
}



NSMomentumWeakStagnationBC::NSMomentumWeakStagnationBC(const std::string & name, InputParameters parameters)
    : NSWeakStagnationBC(name, parameters),

      // Required parameters
      _component(getParam<unsigned>("component"))
{
}




Real NSMomentumWeakStagnationBC::computeQpResidual()
{
  // Compute stagnation values
  Real T_s = 0., p_s = 0., rho_s = 0.;
  this->static_values(T_s, p_s, rho_s);

  // The specified flow direction, as a vector
  RealVectorValue s(_sx, _sy, _sz);

  // (rho_s * |u|^2 * s_k * (s.n) + p_s * n_k) * phi_i
  return (rho_s * this->velmag2() * s(_component) * this->sdotn() + p_s * _normals[_qp](_component)) * _test[_i][_qp];
}




Real NSMomentumWeakStagnationBC::computeQpJacobian()
{
  // TODO
  return 0.;
}




Real NSMomentumWeakStagnationBC::computeQpOffDiagJacobian(unsigned /*jvar*/)
{
  // TODO
  return 0.;
}




