#include "StressDivergence.h"

#include "Material.h"

template<>
InputParameters validParams<StressDivergence>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<Real>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");
  params.addCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");

  params.set<bool>("use_displaced_mesh") = true;

  return params;
}


StressDivergence::StressDivergence(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _stress(getMaterialProperty<RealTensorValue>("stress")),
   _Jacobian_mult(getMaterialProperty<ColumnMajorMatrix>("Jacobian_mult")),
   _component(getParam<Real>("component")),
   _xdisp_coupled(isCoupled("disp_x")),
   _ydisp_coupled(isCoupled("disp_y")),
   _zdisp_coupled(isCoupled("disp_z")),
   _xdisp_var(_xdisp_coupled ? coupled("disp_x") : 0),
   _ydisp_var(_ydisp_coupled ? coupled("disp_y") : 0),
   _zdisp_var(_zdisp_coupled ? coupled("disp_z") : 0)
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
      value(i) += 1.0*_Jacobian_mult[_qp]( (LIBMESH_DIM*_component)+i,(LIBMESH_DIM*_component)+j) * _grad_phi[_j][_qp](j);
      //value(i) += 0.5*_Jacobian_mult[_qp]( _component+(i*LIBMESH_DIM),(LIBMESH_DIM*_component)+j) * _grad_phi[_j][_qp](j);
    }

  return value * _grad_test[_i][_qp];
}

Real
StressDivergence::computeQpOffDiagJacobian(unsigned int jvar)
{
  unsigned int coupled_component = 0;

  bool active(false);

  if ( _xdisp_coupled && jvar == _xdisp_var )
  {
    coupled_component = 0;
    active = true;
  }
  else if ( _ydisp_coupled && jvar == _ydisp_var )
  {
    coupled_component = 1;
    active = true;
  }
  else if ( _zdisp_coupled && jvar == _zdisp_var )
  {
    coupled_component = 2;
    active = true;
  }

  if ( active )
  {
    RealVectorValue value;
    for(unsigned int j = 0; j<LIBMESH_DIM; ++j)
    {
      for(unsigned int i = 0; i<LIBMESH_DIM; ++i)
      {
        value(i) += _Jacobian_mult[_qp]( (LIBMESH_DIM*_component)+i,(LIBMESH_DIM*coupled_component)+j) * _grad_phi[_j][_qp](j);
      }
    }
    return value * _grad_test[_i][_qp];
  }
  return 0;
}
