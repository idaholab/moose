#include "SolidMechanicsMaterial.h"
#include "Problem.h"

#include "VolumetricModel.h"

template<>
InputParameters validParams<SolidMechanicsMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("disp_x", "The x displacement");
  params.addRequiredCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addCoupledVar("temp", "The temperature if you want thermal expansion.");
  params.addCoupledVar("c","variable that zeros out the stiffness");

  return params;
}

SolidMechanicsMaterial::SolidMechanicsMaterial(const std::string & name, InputParameters parameters)
  :Material(name, parameters),
   _grad_disp_x(coupledGradient("disp_x")),
   _grad_disp_y(coupledGradient("disp_y")),
   _grad_disp_z(_dim == 3 ? coupledGradient("disp_z") : _grad_zero),
   _has_temp(isCoupled("temp")),
   _temp(_has_temp ? coupledValue("temp") : _zero),
   _has_c(isCoupled("c")),
   _c( _has_c ? coupledValue("c") : _zero),
   _volumetric_models(0),
   _stress(declareProperty<SymmTensor>("stress")),
   _elasticity_tensor(declareProperty<SymmElasticityTensor>("elasticity_tensor")),
   _Jacobian_mult(declareProperty<SymmElasticityTensor>("Jacobian_mult")),
   _d_strain_dT(),
   _d_stress_dT(declareProperty<SymmTensor>("d_stress_dT")),
   _elastic_strain(declareProperty<SymmTensor>("elastic_strain"))
{}

