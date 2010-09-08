#include "SolidMechX.h"

template<>
InputParameters validParams<SolidMechX>()
{
  InputParameters params = validParams<SolidMech>();
  params.addRequiredCoupledVar("y", "Coupled Displacement in the y Direction");
  params.addCoupledVar("z", "Coupled Displacement in the z Direction");
  return params;
}

SolidMechX::SolidMechX(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :SolidMech(name, moose_system, parameters),
     _y_var(coupled("y")),
     _y(coupledValue("y")),
     _grad_y(coupledGradient("y")),
     _z_var(_dim == 3 ? coupled("z") : 1000000),
     _z(_dim == 3 ? coupledValue("z") : _zero),
     _grad_z(_dim == 3 ? coupledGradient("z"): _grad_zero)
  {}

Real
SolidMechX::computeQpResidual()
  {
    recomputeConstants();

    _strain(0,0) = _grad_u[_qp](0);
    _strain(1,1) = _grad_y[_qp](1);
    _strain(0,1) = _grad_u[_qp](1)+_grad_y[_qp](0);

    _stress(0) = _c1*_strain(0,0)+_c1*_c2*_strain(1,1);
    _stress(1) = _c1*_c3*_strain(0,1);

    if( 3 == _dim){
      _strain(2,2) = _grad_z[_qp](2);
      _strain(0,2) = _grad_u[_qp](2)+_grad_z[_qp](0);

      _stress(0) += _c1*_c2*_strain(2,2);
      _stress(2) = _c1*_c3*_strain(0,2);
    }

    Real value = (_stress*_grad_test[_i][_qp]);
    
    return value;
  }

Real
SolidMechX::computeQpJacobian()
  {
    recomputeConstants();

    Real value = _c1*(_grad_test[_i][_qp]*(_B11*_grad_phi[_j][_qp]));

    return value;
  }

Real
SolidMechX::computeQpOffDiagJacobian(unsigned int jvar)
  {
    recomputeConstants();
    
    RealGradient value = 0;

    if(jvar == _y_var)
      value += _B12*_grad_phi[_j][_qp];
    else if(jvar == _z_var)
      value += _B13*_grad_phi[_j][_qp];

    return _c1*(_grad_test[_i][_qp]*value);
  }
