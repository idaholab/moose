#include "StressDivergenceRZ.h"

#include "Material.h"
#include "SymmElasticityTensor.h"

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
   _stress(getMaterialProperty<SymmTensor>("stress")),
   _Jacobian_mult(getMaterialProperty<SymmElasticityTensor>("Jacobian_mult")),
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
      _grad_test[_i][_qp](0)            * _stress[_qp].xx()
    + _grad_test[_i][_qp](1)            * _stress[_qp].xy()
    + _test[_i][_qp] / _q_point[_qp](0) * _stress[_qp].zz();
// _stress[_qp].row(_component) * _grad_test[_i][_qp];
  }
  else if (_component == 1)
  {
    div =
      _grad_test[_i][_qp](0) * _stress[_qp].xy() +
      _grad_test[_i][_qp](1) * _stress[_qp].yy();
// _stress[_qp].row(_component) * _grad_test[_i][_qp];

  }
  else
  {
    mooseError("Invalid component");
  }

  return 2 * M_PI * _q_point[_qp](0) * div;
}

Real
StressDivergenceRZ::computeQpJacobian()
{
//   ColumnMajorMatrix B(9,1);
//   ColumnMajorMatrix BT(9,1);
  SymmTensor test, phi;
  if (_component == 0)
  {
    test.xx() = _grad_test[_i][_qp](0);
    test.xy() = 0.5*_grad_test[_i][_qp](1);
    test.zz() = _test[_i][_qp] / _q_point[_qp](0);

    phi.xx()  = _grad_phi[_j][_qp](0);
    phi.xy()  = 0.5*_grad_phi[_j][_qp](1);
    phi.zz()  = _test[_j][_qp] / _q_point[_qp](0);
  }
  else
  {
    test.xy() = 0.5*_grad_test[_i][_qp](0);
    test.yy() = _grad_test[_i][_qp](1);

    phi.xy()  = 0.5*_grad_phi[_j][_qp](0);
    phi.yy()  = _grad_phi[_j][_qp](1);
  }

  SymmTensor tmp( _Jacobian_mult[_qp] * test );
  const Real val( phi.doubleContraction( tmp ) );

  return 2 * M_PI * _q_point[_qp](0) * val;
}

Real
StressDivergenceRZ::computeQpOffDiagJacobian(unsigned int /*jvar*/)
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
