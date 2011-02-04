#include "StressDivergenceRZ.h"

#include "Material.h"

template<>
InputParameters validParams<StressDivergenceRZ>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<Real>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for r, 1 for z)");
  params.addCoupledVar("disp_r", "The r displacement");
  params.addCoupledVar("disp_z", "The z displacement");

  params.set<bool>("use_displaced_mesh") = true;

  return params;
}


StressDivergenceRZ::StressDivergenceRZ(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _stress(getMaterialProperty<RealTensorValue>("stress")),
   _Jacobian_mult(getMaterialProperty<ColumnMajorMatrix>("Jacobian_mult")),
   _component(getParam<Real>("component")),
   _rdisp_coupled(isCoupled("disp_r")),
   _zdisp_coupled(isCoupled("disp_z")),
   _rdisp_var(_rdisp_coupled ? coupled("disp_r") : 0),
   _zdisp_var(_zdisp_coupled ? coupled("disp_z") : 0)
{}

Real
StressDivergenceRZ::computeQpResidual()
{
  Real div(0);
  if (_component == 0)
  {
    div =
      _grad_test[_i][_qp](0)            * _stress[_qp](0,0)
    + _grad_test[_i][_qp](1)            * _stress[_qp](1,0)
    + _test[_i][_qp] / _q_point[_qp](0) * _stress[_qp](2,2);
// _stress[_qp].row(_component) * _grad_test[_i][_qp];
  }
  else if (_component == 1)
  {
    div =
      _grad_test[_i][_qp](0) * _stress[_qp](0,1) +
      _grad_test[_i][_qp](1) * _stress[_qp](1,1);
// _stress[_qp].row(_component) * _grad_test[_i][_qp];

//     std::cout << "JDH DEBUG: "
//               << _grad_test[_i][_qp](0) << " "
//               << _stress[_qp](0,1) << std::endl;
  }
  else
  {
    mooseError("Invalid component");
  }

//   return 2 * M_PI * _q_point[_qp](0) * div;
//   std::cout << "JDH DEBUG: q_point: " << _q_point[_qp](0) << std::endl;
  return 2 * M_PI * div * _q_point[_qp](0);
}

Real
StressDivergenceRZ::computeQpJacobian()
{
  ColumnMajorMatrix B(9,1);
  ColumnMajorMatrix BT(9,1);
  if (_component == 0)
  {
    B(0,0) = _grad_phi[_j][_qp](0);
    B(1,0) = 0.5*_grad_phi[_j][_qp](1);
    B(3,0) = 0.5*_grad_phi[_j][_qp](1);
    B(8,0) = _phi[_j][_qp] / _q_point[_qp](0);
    BT(0,0) = _grad_test[_i][_qp](0);
    BT(1,0) = 0.5*_grad_test[_i][_qp](1);
    BT(3,0) = 0.5*_grad_test[_i][_qp](1);
    BT(8,0) = _test[_i][_qp] / _q_point[_qp](0);
  }
  else
  {
    B(1,0) = 0.5*_grad_phi[_j][_qp](0);
    B(3,0) = 0.5*_grad_phi[_j][_qp](0);
    B(4,0) = _grad_phi[_j][_qp](1);
    BT(1,0) = 0.5*_grad_test[_i][_qp](0);
    BT(3,0) = 0.5*_grad_test[_i][_qp](0);
    BT(4,0) = _grad_test[_i][_qp](1);
  }

  ColumnMajorMatrix temp(_Jacobian_mult[_qp] * B);
  Real result(0);
  for (unsigned i(0); i < 9; ++i)
  {
    result += BT(i,0) * temp(i,0);
  }
  return 2 * M_PI * _q_point[_qp](0) * result;
}

Real
StressDivergenceRZ::computeQpOffDiagJacobian(unsigned int jvar)
{
//   unsigned int coupled_component = 0;

//   bool active(false);

//   if ( _rdisp_coupled && jvar == _rdisp_var )
//   {
//     coupled_component = 0;
//     active = true;
//   }
//   else if ( _zdisp_coupled && jvar == _zdisp_var )
//   {
//     coupled_component = 2;
//     active = true;
//   }

//   if ( active )
//   {
//     RealVectorValue value;
//     for(unsigned int j = 0; j<LIBMESH_DIM; ++j)
//     {
//       for(unsigned int i = 0; i<LIBMESH_DIM; ++i)
//       {
//         value(i) += _Jacobian_mult[_qp]( (LIBMESH_DIM*_component)+i,(LIBMESH_DIM*coupled_component)+j) * _grad_phi[_j][_qp](j);
//       }
//     }
//     return value * _grad_test[_i][_qp];
//   }
  return 0;
}
