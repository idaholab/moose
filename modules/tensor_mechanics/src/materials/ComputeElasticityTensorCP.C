/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeElasticityTensorCP.h"
#include "RotationTensor.h"

template<>
InputParameters validParams<ComputeElasticityTensorCP>()
{
  InputParameters params = validParams<ComputeElasticityTensor>();
  params.addClassDescription("Compute an elasticity tensor for crystal plasticity.");
  params.addParam<UserObjectName>("read_prop_user_object","The ElementReadPropertyFile GeneralUserObject to read element specific property values from file");
  return params;
}

ComputeElasticityTensorCP::ComputeElasticityTensorCP(const InputParameters & parameters) :
    ComputeElasticityTensor(parameters),
    _read_prop_user_object(isParamValid("read_prop_user_object") ? & getUserObject<ElementPropertyReadFile>("read_prop_user_object") : NULL),
    _local_Euler_angles(declareProperty<RealVectorValue>("Euler_angles")),
    _crysrot(declareProperty<RankTwoTensor>("crysrot")),
    _R(_Euler_angles)
{
}

void
ComputeElasticityTensorCP::computeQpElasticityTensor()
{
  //Properties assigned at the beginning of every call to material calculation
  if ( _read_prop_user_object )
  {
    _local_Euler_angles[_qp](0) = _read_prop_user_object->getData( _current_elem , 0 );
    _local_Euler_angles[_qp](1) = _read_prop_user_object->getData( _current_elem , 1 );
    _local_Euler_angles[_qp](2) = _read_prop_user_object->getData( _current_elem , 2 );
  }

  RealVectorValue euler_angles;
  euler_angles(0) = _local_Euler_angles[_qp](0);
  euler_angles(1) = _local_Euler_angles[_qp](1);
  euler_angles(2) = _local_Euler_angles[_qp](2);

  _R.update(euler_angles);

  _crysrot[_qp] = _R.transpose();

  _elasticity_tensor[_qp] = _Cijkl;
  _elasticity_tensor[_qp].rotate(_crysrot[_qp]);
}
