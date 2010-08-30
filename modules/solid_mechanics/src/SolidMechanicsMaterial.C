#include "SolidMechanicsMaterial.h"

template<>
InputParameters validParams<SolidMechanicsMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("disp_x", "The x displacement");
  params.addRequiredCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addCoupledVar("temp", "The temperature if you want thermal expansion.");
  return params;
}

SolidMechanicsMaterial::SolidMechanicsMaterial(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Material(name, moose_system, parameters),
   _grad_disp_x(coupledGradient("disp_x")),
   _grad_disp_y(coupledGradient("disp_y")),
   _grad_disp_z(_dim == 3 ? coupledGradient("disp_z") : _grad_zero),
   _has_temp(isCoupled("temp")),
   _temp(_has_temp ? coupledValue("temp") : _zero),
   _stress(declareProperty<RealTensorValue>("stress")),
   _elasticity_tensor(declareProperty<ColumnMajorMatrix>("elasticity_tensor")),
   _elastic_strain(declareProperty<ColumnMajorMatrix>("elastic_strain")),
   _thermal_conductivity(declareProperty<Real>("thermal_conductivity")),
   _density(declareProperty<Real>("density")),
   _specific_heat(declareProperty<Real>("specific_heat"))
{}
