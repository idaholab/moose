#include "LinearAnisotropicMaterial.h"

// Elk Includes
#include "ColumnMajorMatrix.h"
#include "AnisotropicElasticityTensor.h"
#include "LinearIsotropicMaterial.h"

template<>
InputParameters validParams<LinearAnisotropicMaterial>()
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

LinearAnisotropicMaterial::LinearAnisotropicMaterial(const std::string & name,
                                                 InputParameters parameters)
  :LinearIsotropicMaterial(name, parameters),
   _material_constant_c11(getParam<Real>("material_constant_c11")),
   _material_constant_c12(getParam<Real>("material_constant_c12")),
   _material_constant_c44(getParam<Real>("material_constant_c44")),
   _euler_angle_1(getParam<Real>("euler_angle_1")),
   _euler_angle_2(getParam<Real>("euler_angle_2")),
   _euler_angle_3(getParam<Real>("euler_angle_3"))
{
  AnisotropicElasticityTensor * aniso_elasticity_tensor = new AnisotropicElasticityTensor;
  aniso_elasticity_tensor->setMaterialConstantc11(_material_constant_c11);
  aniso_elasticity_tensor->setMaterialConstantc12(_material_constant_c12);
  aniso_elasticity_tensor->setMaterialConstantc44(_material_constant_c44);
  aniso_elasticity_tensor->setFirstEulerAngle(_euler_angle_1);
  aniso_elasticity_tensor->setSecondEulerAngle(_euler_angle_2);
  aniso_elasticity_tensor->setThirdEulerAngle(_euler_angle_3);

  _local_elasticity_tensor = aniso_elasticity_tensor;
}
