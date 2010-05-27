#include "HeatConduction.h"

template<>
InputParameters validParams<HeatConduction>()
{
  InputParameters params = validParams<Diffusion>();
  return params;
}

HeatConduction::HeatConduction(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Diffusion(name, moose_system, parameters),
   _k(_material_data.getRealProperty("thermal_conductivity"))
  {}

Real
HeatConduction::computeQpResidual()
{
  return _k[_qp]*Diffusion::computeQpResidual();
}

Real
HeatConduction::computeQpJacobian()
{
  return _k[_qp]*Diffusion::computeQpJacobian();
}
