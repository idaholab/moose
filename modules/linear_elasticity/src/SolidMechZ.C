#include "SolidMechZ.h"

template<>
Parameters valid_params<SolidMechZ>()
{
  return valid_params<SolidMech>();
}

SolidMechZ::SolidMechZ(std::string name,
             Parameters parameters,
             std::string var_name,
             std::vector<std::string> coupled_to,
             std::vector<std::string> coupled_as)
    :SolidMech(name,parameters,var_name,coupled_to,coupled_as),
    _x_var(coupled("x")),
    _x(coupledVal("x")),
    _grad_x(coupledGrad("x")),
    _y_var(coupled("y")),
    _y(coupledVal("y")),
    _grad_y(coupledGrad("y"))
  {}

Real
SolidMechZ::computeQpResidual()
  {
    recomputeConstants();

    _strain(0,0) = _grad_x[_qp](0);
    _strain(1,1) = _grad_y[_qp](1);
    _strain(2,2) = _grad_u[_qp](2);
    _strain(1,2) = _grad_u[_qp](1)+_grad_y[_qp](2);
    _strain(0,2) = _grad_x[_qp](2)+_grad_u[_qp](0);

    _stress(0) = _c1*_c3*_strain(0,2);
    _stress(1) = _c1*_c3*_strain(1,2);
    _stress(2) = _c1*_c2*_strain(0,0)+_c1*_c2*_strain(1,1)+_c1*_strain(2,2);

    return (_stress*_dphi[_i][_qp]);

  }

Real
SolidMechZ::computeQpJacobian()
  {
    recomputeConstants();

    Real value = _c1*(_dphi[_i][_qp]*(_B33*_dphi[_j][_qp]));

    return value;
  }

Real
SolidMechZ::computeQpOffDiagJacobian(unsigned int jvar)
  {
    recomputeConstants();
    
    RealGradient value = 0;

    if(jvar == _x_var)
      value += _B31*_dphi[_j][_qp];
    else if(jvar == _y_var)
      value += _B32*_dphi[_j][_qp];

    return _c1*(_dphi[_i][_qp]*value);
  }
