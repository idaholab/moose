#include "SolidMechanicsMaterial.h"

template<>
InputParameters validParams<SolidMechanicsMaterial>()
{
  InputParameters params = validParams<Material>();
  return params;
}

SolidMechanicsMaterial::SolidMechanicsMaterial(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Material(name, moose_system, parameters),
   _grad_disp_x(coupledGrad("disp_x")),
   _grad_disp_y(coupledGrad("disp_y")),
   _grad_disp_z(_dim == 3 ? coupledGrad("disp_z") : _grad_zero),
   _has_temp(isCoupled("temp")),
   _temp(_has_temp ? coupledVal("temp") : _zero),
   _stress(declareTensorProperty("stress")),
   _elasticity_tensor(declareColumnMajorMatrixProperty("elasticity_tensor"))
{}
