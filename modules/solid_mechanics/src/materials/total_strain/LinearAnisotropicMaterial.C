/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "LinearAnisotropicMaterial.h"
#include "SymmAnisotropicElasticityTensor.h"

template <>
InputParameters
validParams<LinearAnisotropicMaterial>()
{

  InputParameters params = validParams<LinearIsotropicMaterial>();
  params.addRequiredParam<Real>("material_constant_c11", "Material modulus C11");
  params.addRequiredParam<Real>("material_constant_c12", "Material modulus C12");
  params.addRequiredParam<Real>("material_constant_c44", "Material modulus C44");
  params.addParam<Real>("euler_angle_1", 0.0, "Euler angle in direction 1");
  params.addParam<Real>("euler_angle_2", 0.0, "Euler angle in direction 2");
  params.addParam<Real>("euler_angle_3", 0.0, "Euler angle in direction 3");
  return params;
}

LinearAnisotropicMaterial::LinearAnisotropicMaterial(const InputParameters & parameters)
  : LinearIsotropicMaterial(parameters),
    _material_constant_c11(getParam<Real>("material_constant_c11")),
    _material_constant_c12(getParam<Real>("material_constant_c12")),
    _material_constant_c44(getParam<Real>("material_constant_c44")),
    _euler_angle_1(getParam<Real>("euler_angle_1")),
    _euler_angle_2(getParam<Real>("euler_angle_2")),
    _euler_angle_3(getParam<Real>("euler_angle_3"))
{
  SymmAnisotropicElasticityTensor * aniso_elasticity_tensor = new SymmAnisotropicElasticityTensor;
  aniso_elasticity_tensor->setMaterialConstantc11(_material_constant_c11);
  aniso_elasticity_tensor->setMaterialConstantc12(_material_constant_c12);
  aniso_elasticity_tensor->setMaterialConstantc44(_material_constant_c44);

  /* AMJ: I believe I have identified a logic bug with using inherited classes (e.g.,
     LinearAnisotropicMaterial and non-zero Euler angles in conjunction with/inheriting from
     LinearIsotropicMaterial.  When using Euler angles = 0.0, no problem occurs.  However,
     using Euler angles != zero causes the _elasticity_tensor[_qp]=*_local_elasticity_tensor
     to rotate with every single quadrature point, every time the Material class is
     computed.  This is due to the _local_elasticity_tensor->calculate(_qp) call.  Because
     we are dereferencing the _local_elasticity_tensor, we are actually changing the
     original elasticity tensor that is supplied (from the input or material class
     construction, etc).  I've attempted to fix this by moving the relevant information
     into a local copy_local_tensor variable, but (perhaps because I don't know enough
     C++) I cannot get this to work in LinearIsotropicMaterial for an arbitrary derived class that
     may use different derived kinds of Symm***ElasticityTensors.  So... good luck, Chuck.
     I might not be able to fix the problem, but I think I've identified it correctly. */

  aniso_elasticity_tensor->setFirstEulerAngle(_euler_angle_1);
  aniso_elasticity_tensor->setSecondEulerAngle(_euler_angle_2);
  aniso_elasticity_tensor->setThirdEulerAngle(_euler_angle_3);

  delete _local_elasticity_tensor;
  _local_elasticity_tensor = aniso_elasticity_tensor;
}
