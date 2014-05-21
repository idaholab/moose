#include "MomentBalancing.h"

#include "Material.h"
#include "ElasticityTensorR4.h"
#include "RankTwoTensor.h"

template<>
InputParameters validParams<MomentBalancing>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<unsigned int>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");
  params.addParam<std::string>("appended_property_name", "", "Name appended to material properties to make them unique");
  params.addCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.set<bool>("use_displaced_mesh") = false;

  return params;
}


MomentBalancing::MomentBalancing(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _stress(getMaterialProperty<RankTwoTensor>("stress" + getParam<std::string>("appended_property_name"))),
   _Jacobian_mult(getMaterialProperty<ElasticityTensorR4>("Jacobian_mult" + getParam<std::string>("appended_property_name"))),
   _component(getParam<unsigned int>("component")),
   _xdisp_coupled(isCoupled("disp_x")),
   _ydisp_coupled(isCoupled("disp_y")),
   _zdisp_coupled(isCoupled("disp_z")),
   _xdisp_var(_xdisp_coupled ? coupled("disp_x") : 0),
   _ydisp_var(_ydisp_coupled ? coupled("disp_y") : 0),
   _zdisp_var(_zdisp_coupled ? coupled("disp_z") : 0)
{}

Real
MomentBalancing::computeQpResidual()
{
  Real the_sum = 0;
  for (unsigned int j = 0 ; j < LIBMESH_DIM ; ++j)
    for (unsigned int k = 0 ; k < LIBMESH_DIM ; ++k)
      the_sum += PermutationTensor::eps(_component, j, k)*_stress[_qp](j, k);
  return -_test[_i][_qp]*the_sum;
}

Real
MomentBalancing::computeQpJacobian()
{
  return _Jacobian_mult[_qp].momentJacobian( _component, _component, _test[_i][_qp], _grad_phi[_j][_qp] );
}

Real
MomentBalancing::computeQpOffDiagJacobian(unsigned int jvar)
{
  unsigned int coupled_component = 0;
  bool active = false;

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

  if (!active)
    return 0.0;

  return _Jacobian_mult[_qp].momentJacobian( _component, coupled_component, _test[_i][_qp], _grad_phi[_j][_qp] );
}
