#include "ComputeStrainBase.h"

template<>
InputParameters validParams<ComputeStrainBase>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("disp_x", "The x displacement");
  params.addRequiredCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addParam<Real>("temperature_ref", 273, "Reference temperature for thermal expansion in K");
  params.addCoupledVar("temperature", 273, "temperature in Kelvin");
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<Real>("thermal_expansion_coeff", 0, "Thermal expansion coefficient in 1/K");
  return params;
}

ComputeStrainBase::ComputeStrainBase(const std::string & name,
                                                 InputParameters parameters) :
    DerivativeMaterialInterface<Material>(name, parameters),
    _grad_disp_x(coupledGradient("disp_x")),
    _grad_disp_y(coupledGradient("disp_y")),
    _grad_disp_z(_mesh.dimension() == 3 ? coupledGradient("disp_z") : _grad_zero),
    _grad_disp_x_old(_fe_problem.isTransient() ? coupledGradientOld("disp_x") : _grad_zero),
    _grad_disp_y_old(_fe_problem.isTransient() ? coupledGradientOld("disp_y") : _grad_zero),
    _grad_disp_z_old(_fe_problem.isTransient() && _mesh.dimension() == 3 ? coupledGradientOld("disp_z") : _grad_zero),
    _T(coupledValue("temperature")),
    _T0(getParam<Real>("temperature_ref")),
    _thermal_expansion_coeff(getParam<Real>("thermal_expansion_coeff")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : "" )
{
}
