/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "SolidMechY.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<SolidMechY>()
{
  InputParameters params = validParams<SolidMech>();
  params.addRequiredCoupledVar("x", "Coupled Displacement in the x Direction");
  params.addCoupledVar("z", "Coupled Displacement in the z Direction");
  return params;
}

SolidMechY::SolidMechY(const InputParameters & parameters) :
    SolidMech(parameters),
    _mesh_dimension(_mesh.dimension()),
    _x_var(coupled("x")),
    _x(coupledValue("x")),
    _grad_x(coupledGradient("x")),
    _z_var(_mesh_dimension == 3 ? coupled("z") : 1000000),
    _z(_mesh_dimension == 3 ? coupledValue("z") : _zero),
    _grad_z(_mesh_dimension == 3 ? coupledGradient("z") : _grad_zero)
  {}

Real
SolidMechY::computeQpResidual()
  {
    if (!_constant_properties)
      recomputeConstants();

    _strain(0,0) = _grad_x[_qp](0);
    _strain(1,1) = _grad_u[_qp](1);
    _strain(0,1) = _grad_x[_qp](1)+_grad_u[_qp](0);

    _stress(0) = _c1*_c3*_strain(0,1);
    _stress(1) = _c1*_c2*_strain(0,0)+_c1*_strain(1,1);

    if ( 3 == _mesh_dimension){
      _strain(2,2) = _grad_z[_qp](2);
      _strain(1,2) = _grad_z[_qp](1)+_grad_u[_qp](2);

      _stress(1) += _c1*_c2*_strain(2,2);
      _stress(2) = _c1*_c3*_strain(1,2);
    }

    Real value = (_stress*_grad_test[_i][_qp]);

    return value;
  }

Real
SolidMechY::computeQpJacobian()
  {
    if (!_constant_properties)
      recomputeConstants();

    Real value = _c1*(_grad_test[_i][_qp]*(_B22*_grad_phi[_j][_qp]));

    return value;
  }

Real
SolidMechY::computeQpOffDiagJacobian(unsigned int jvar)
  {
    if (!_constant_properties)
      recomputeConstants();

    RealGradient value = 0;

    if (jvar == _x_var)
      value += _B21*_grad_phi[_j][_qp];
    else if (jvar == _z_var)
      value += _B23*_grad_phi[_j][_qp];

    return _c1*(_grad_test[_i][_qp]*value);
  }

