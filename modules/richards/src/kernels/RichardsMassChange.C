#include "RichardsMassChange.h"
#include "Material.h"

#include <iostream>


template<>
InputParameters validParams<RichardsMassChange>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<bool>("lumping", false, "True for mass matrix lumping, false otherwise.  NOTE: THIS CURRENTLY DOES NOTHING!");
  params.addParam<bool>("use_supg", false, "True for using SUPG in this kernel, false otherwise.  This has no effect if the material does not use SUPG.");
  return params;
}

RichardsMassChange::RichardsMassChange(const std::string & name,
                                             InputParameters parameters) :
    Kernel(name,parameters),

    _lumping(getParam<bool>("lumping")),
    _use_supg(getParam<bool>("use_supg")),
    // This kernel expects input parameters named "bulk_mod", etc
    _porosity(getMaterialProperty<Real>("porosity")),

    _sat_old(getMaterialProperty<Real>("sat_old")),

    _sat(getMaterialProperty<Real>("sat")),
    _dsat(getMaterialProperty<Real>("dsat")),
    _d2sat(getMaterialProperty<Real>("d2sat")),

    _density_old(getMaterialProperty<Real>("density_old")), 

    _density(getMaterialProperty<Real>("density")), 
    _ddensity(getMaterialProperty<Real>("ddensity")),
    _d2density(getMaterialProperty<Real>("d2density")),

    _vel_SUPG(getMaterialProperty<RealVectorValue>("vel_SUPG")),
    _vel_prime_SUPG(getMaterialProperty<RealTensorValue>("vel_prime_SUPG")),
    _tau_SUPG(getMaterialProperty<Real>("tau_SUPG")),
    _tau_prime_SUPG(getMaterialProperty<RealVectorValue>("tau_prime_SUPG"))
{
}


Real
RichardsMassChange::computeQpResidual()
{
  Real mass = _porosity[_qp]*_density[_qp]*_sat[_qp];
  Real mass_old = _porosity[_qp]*_density_old[_qp]*_sat_old[_qp];

  Real test_fcn = _test[_i][_qp] ;
  if (_use_supg) {
    test_fcn += _tau_SUPG[_qp]*_vel_SUPG[_qp]*_grad_test[_i][_qp];
  }

  return test_fcn*(mass - mass_old)/_dt;
}

Real
RichardsMassChange::computeQpJacobian()
{
  Real mass = _porosity[_qp]*_density[_qp]*_sat[_qp];
  Real mass_old = _porosity[_qp]*_density_old[_qp]*_sat_old[_qp];
  Real mass_prime = _phi[_j][_qp]*_porosity[_qp]*(_ddensity[_qp]*_sat[_qp] + _density[_qp]*_dsat[_qp]);

  Real test_fcn = _test[_i][_qp] ;
  Real test_fcn_prime = 0;
    
  if (_use_supg) {
    test_fcn += _tau_SUPG[_qp]*_vel_SUPG[_qp]*_grad_test[_i][_qp];
    test_fcn_prime += ((_tau_prime_SUPG[_qp]*_grad_phi[_j][_qp])*(_vel_SUPG[_qp]*_grad_test[_i][_qp]) + _tau_SUPG[_qp]*(_vel_prime_SUPG[_qp]*_grad_phi[_j][_qp])*_grad_test[_i][_qp]);
  }

  return (test_fcn*mass_prime + test_fcn_prime*(mass- mass_old))/_dt;
}

/*
void
RichardsMassChange::computeJacobian()
{
  if (_lumping)
  {
    DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.index(), _var.index());

    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        {
          ke(_i, _i) += _JxW[_qp] * _coord[_qp] * computeQpJacobian();
        }
  }
  else
    Kernel::computeJacobian();
}
*/
