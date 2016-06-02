/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "SolidMechanicsMaterial.h"
#include "Problem.h"
#include "VolumetricModel.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<SolidMechanicsMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addParam<std::string>("appended_property_name", "", "Name appended to material properties to make them unique");
  params.addRequiredCoupledVar("disp_x", "The x displacement");
  params.addRequiredCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addCoupledVar("temp", "The temperature if you want thermal expansion.");
  params.addCoupledVar("c","variable that zeros out the stiffness");

  return params;
}

void
SolidMechanicsMaterial::initQpStatefulProperties()
{
}

SolidMechanicsMaterial::SolidMechanicsMaterial(const InputParameters & parameters)
  :Material(parameters),
   _appended_property_name( getParam<std::string>("appended_property_name") ),
   _grad_disp_x(coupledGradient("disp_x")),
   _grad_disp_y(coupledGradient("disp_y")),
   _grad_disp_z(_mesh.dimension() == 3 ? coupledGradient("disp_z") : _grad_zero),
   _has_temp(isCoupled("temp")),
   _temp(_has_temp ? coupledValue("temp") : _zero),
   _has_c(isCoupled("c")),
   _c( _has_c ? coupledValue("c") : _zero),
   _volumetric_models(0),
   _stress(createProperty<SymmTensor>("stress")),
   _elasticity_tensor(createProperty<SymmElasticityTensor>("elasticity_tensor")),
   _Jacobian_mult(createProperty<SymmElasticityTensor>("Jacobian_mult")),
   _d_strain_dT(),
   _d_stress_dT(createProperty<SymmTensor>("d_stress_dT")),
   _elastic_strain(createProperty<SymmTensor>("elastic_strain"))
{}
