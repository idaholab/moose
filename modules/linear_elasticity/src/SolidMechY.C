#include "SolidMechY.h"

template<>
InputParameters valid_params<SolidMechY>()
{
  return valid_params<SolidMech>();
}

SolidMechY::SolidMechY(std::string name,
             InputParameters parameters,
             std::string var_name,
             std::vector<std::string> coupled_to,
             std::vector<std::string> coupled_as)
    :SolidMech(name,parameters,var_name,coupled_to,coupled_as),
    _x_var(coupled("x")),
    _x(coupledVal("x")),
    _grad_x(coupledGrad("x")),
    _z_var(_dim == 3 ? coupled("z") : 1000000),
    _z(_dim == 3 ? coupledVal("z") : _zero),
    _grad_z(_dim == 3 ? coupledGrad("z") : _grad_zero)
  {}

Real
SolidMechY::computeQpResidual()
  {
    recomputeConstants();

    _strain(0,0) = _grad_x[_qp](0);
    _strain(1,1) = _grad_u[_qp](1);
    _strain(0,1) = _grad_x[_qp](1)+_grad_u[_qp](0);

    _stress(0) = _c1*_c3*_strain(0,1);
    _stress(1) = _c1*_c2*_strain(0,0)+_c1*_strain(1,1);

    if( 3 == _dim){
      _strain(2,2) = _grad_z[_qp](2);
      _strain(1,2) = _grad_z[_qp](1)+_grad_u[_qp](2);

      _stress(1) += _c1*_c2*_strain(2,2);
      _stress(2) = _c1*_c3*_strain(1,2);
    }

    Real value = (_stress*_dphi[_i][_qp]);
    
    return value;
  }

Real
SolidMechY::computeQpJacobian()
  {
    recomputeConstants();

    Real value = _c1*(_dphi[_i][_qp]*(_B22*_dphi[_j][_qp]));

    return value;
  }

Real
SolidMechY::computeQpOffDiagJacobian(unsigned int jvar)
  {
    recomputeConstants();
    
    RealGradient value = 0;

    if(jvar == _x_var)
      value += _B21*_dphi[_j][_qp];
    else if(jvar == _z_var)
      value += _B23*_dphi[_j][_qp];

    return _c1*(_dphi[_i][_qp]*value);
  }
