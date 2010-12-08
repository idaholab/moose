#include "StressDivergence.h"

#include "Material.h"

template<>
InputParameters validParams<StressDivergence>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<Real>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");

  params.set<bool>("use_displaced_mesh") = false;

  return params;
}


StressDivergence::StressDivergence(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _stress(getMaterialProperty<RealTensorValue>("stress")),
   _Jacobian_mult(getMaterialProperty<ColumnMajorMatrix>("Jacobian_mult")),
   _component(getParam<Real>("component")),
   _xdisp_var(isCoupled("x_disp") ? coupled("x_disp") : 0),
   _ydisp_var(isCoupled("y_disp") ? coupled("y_disp") : 0),
   _zdisp_var(isCoupled("z_disp") ? coupled("z_disp") : 0)
{}

Real
StressDivergence::computeQpResidual()
{
  return _stress[_qp].row(_component) * _grad_test[_i][_qp];
}

Real
StressDivergence::computeQpJacobian()
{
  RealVectorValue value;
  for(unsigned int j = 0; j<LIBMESH_DIM; ++j)
    for(unsigned int i = 0; i<LIBMESH_DIM; ++i)
    {
      value(i) += 0.5*_Jacobian_mult[_qp]( (LIBMESH_DIM*_component)+i,(LIBMESH_DIM*_component)+j) * _grad_phi[_j][_qp](j);
      value(i) += 0.5*_Jacobian_mult[_qp]( _component+(i*LIBMESH_DIM),(LIBMESH_DIM*_component)+j) * _grad_phi[_j][_qp](j);
    }

  return value * _grad_test[_i][_qp];
}

Real
StressDivergence::computeQpOffDiagJacobian(unsigned int jvar)
{
  unsigned int coupled_component = 0;

  if(jvar == _ydisp_var)
    coupled_component = 1;
  else if(jvar == _zdisp_var)
    coupled_component = 2;

  RealVectorValue value;
  for(unsigned int j = 0; j<LIBMESH_DIM; j++)
    for(unsigned int i = 0; i<LIBMESH_DIM; i++)
      value(i) += _Jacobian_mult[_qp]( (LIBMESH_DIM*_component)+i,(LIBMESH_DIM*coupled_component)+j) * _grad_phi[_j][_qp](j);

  return value * _grad_test[_i][_qp];
}
