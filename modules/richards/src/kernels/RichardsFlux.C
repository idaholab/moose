#include "RichardsFlux.h"
#include "Material.h"

#include <iostream>


template<>
InputParameters validParams<RichardsFlux>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<bool>("linear_shape_fcns", true, "If you are using second-order Lagrange shape functions you need to set this to false.");
  return params;
}

RichardsFlux::RichardsFlux(const std::string & name,
                                             InputParameters parameters) :
    Kernel(name,parameters),
    // This kernel gets lots of things from the material
    _dens0(getMaterialProperty<Real>("dens0")),
    _viscosity(getMaterialProperty<Real>("viscosity")),
    _gravity(getMaterialProperty<RealVectorValue>("gravity")),
    _permeability(getMaterialProperty<RealTensorValue>("permeability")),

    _seff(getMaterialProperty<Real>("s_eff")), // not actually used
    _dseff(getMaterialProperty<Real>("ds_eff")),
    _d2seff(getMaterialProperty<Real>("d2s_eff")),

    _rel_perm(getMaterialProperty<Real>("rel_perm")),
    _drel_perm(getMaterialProperty<Real>("drel_perm")),
    _d2rel_perm(getMaterialProperty<Real>("d2rel_perm")),

    _density(getMaterialProperty<Real>("density")), 
    _ddensity(getMaterialProperty<Real>("ddensity")),
    _d2density(getMaterialProperty<Real>("d2density")),

    _second_u(getParam<bool>("linear_shape_fcns") ? _second_zero : (_is_implicit ? _var.secondSln() : _var.secondSlnOld())),
    //_second_phi(getParam<bool>("linear_shape_fcns") ? _second_zero : secondPhi()), // does not compile
    _second_phi(secondPhi()), // does compile

    _vel_SUPG(getMaterialProperty<RealVectorValue>("vel_SUPG")),
    _vel_prime_SUPG(getMaterialProperty<RealTensorValue>("vel_prime_SUPG")),
    _tau_SUPG(getMaterialProperty<Real>("tau_SUPG")),
    _tau_prime_SUPG(getMaterialProperty<RealVectorValue>("tau_prime_SUPG"))

{}


Real
RichardsFlux::computeQpResidual()
{

  // std::cout << _permeability[_qp];

  Real flux_part = _grad_test[_i][_qp]*((_density[_qp]*_rel_perm[_qp]/_viscosity[_qp])*(_permeability[_qp]*(_grad_u[_qp] - _dens0[_qp]*_gravity[_qp])));
  Real supg_test = _tau_SUPG[_qp]*_vel_SUPG[_qp]*_grad_test[_i][_qp];
  Real supg_kernel = 0.0;

  if (_tau_SUPG[_qp] != 0)
    {
      supg_kernel = -(_ddensity[_qp]*_rel_perm[_qp] + _density[_qp]*_drel_perm[_qp]*_dseff[_qp])/_viscosity[_qp]*_grad_u[_qp]*(_permeability[_qp]*(_grad_u[_qp] - _dens0[_qp]*_gravity[_qp]));
      // NOTE: since Libmesh does not correctly calculate grad(_grad_u) correctly, so following might not be correct
      supg_kernel -= (_density[_qp]*_rel_perm[_qp]/_viscosity[_qp])*((_permeability[_qp]*_second_u[_qp]).tr());
    }
  return flux_part + supg_test*supg_kernel;
}


Real
RichardsFlux::computeQpJacobian()
{
  Real flux_prime = _grad_test[_i][_qp]*((_density[_qp]*_rel_perm[_qp]/_viscosity[_qp])*(_permeability[_qp]*(_grad_phi[_j][_qp]))) + _grad_test[_i][_qp]*((_ddensity[_qp]*_rel_perm[_qp] + _density[_qp]*_drel_perm[_qp]*_dseff[_qp])*_phi[_j][_qp]/_viscosity[_qp]*(_permeability[_qp]*(_grad_u[_qp] - _dens0[_qp]*_gravity[_qp])));

  Real supg_test = _tau_SUPG[_qp]*_vel_SUPG[_qp]*_grad_test[_i][_qp];
  Real supg_test_prime = (_tau_prime_SUPG[_qp]*_grad_phi[_j][_qp])*(_vel_SUPG[_qp]*_grad_test[_i][_qp]) + _tau_SUPG[_qp]*(_vel_prime_SUPG[_qp]*_grad_phi[_j][_qp])*_grad_test[_i][_qp];
  Real supg_kernel = 0.0;
  Real supg_kernel_prime = 0.0;
  if (_tau_SUPG[_qp] != 0)
    {
      supg_kernel = -(_ddensity[_qp]*_rel_perm[_qp] + _density[_qp]*_drel_perm[_qp]*_dseff[_qp])/_viscosity[_qp]*_grad_u[_qp]*(_permeability[_qp]*(_grad_u[_qp] - _dens0[_qp]*_gravity[_qp]));
      supg_kernel -= (_density[_qp]*_rel_perm[_qp]/_viscosity[_qp])*((_permeability[_qp]*_second_u[_qp]).tr());

      supg_kernel_prime = -(_d2density[_qp]*_rel_perm[_qp] + 2*_ddensity[_qp]*_drel_perm[_qp]*_dseff[_qp] + _density[_qp]*_d2rel_perm[_qp]*_dseff[_qp]*_dseff[_qp] + _density[_qp]*_drel_perm[_qp]*_d2seff[_qp])*_phi[_j][_qp]/_viscosity[_qp]*_grad_u[_qp]*(_permeability[_qp]*(_grad_u[_qp] - _dens0[_qp]*_gravity[_qp]));
      supg_kernel_prime -= (_ddensity[_qp]*_rel_perm[_qp] + _density[_qp]*_drel_perm[_qp]*_dseff[_qp])/_viscosity[_qp]*(_grad_phi[_j][_qp]*(_permeability[_qp]*(_grad_u[_qp] - _dens0[_qp]*_gravity[_qp])) + _grad_u[_qp]*(_permeability[_qp]*_grad_phi[_j][_qp]));
      supg_kernel_prime -= ((_ddensity[_qp]*_rel_perm[_qp] + _density[_qp]*_drel_perm[_qp]*_dseff[_qp])*_phi[_j][_qp]/_viscosity[_qp]*(_permeability[_qp]*_second_u[_qp]).tr());
      //supg_kernel_prime -= (_density[_qp]*_rel_perm[_qp]/_viscosity[_qp])*((_permeability[_qp]*_second_phi[_j][_qp]).tr()); //uncomment once second_phi is zeroed
    }

  //std::cout << "elem=" << _current_elem->id() << " qp=" << _qp << " gradtest=" << _grad_test[_i][_qp] << " supg_test=" << supg_test << " supg_test_prime=" << supg_test_prime << "\n";
      

  return flux_prime + (supg_test_prime*supg_kernel + supg_test*supg_kernel_prime);

}
