#include "HomogenizationHeatConduction.h"

template<>
InputParameters validParams<HomogenizationHeatConduction>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<unsigned int>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");

  return params;
}


HomogenizationHeatConduction::HomogenizationHeatConduction(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _thermal_conductivity(getMaterialProperty<Real>("thermal_conductivity")),
   _component(getParam<unsigned int>("component"))
{}

Real
HomogenizationHeatConduction::computeQpResidual()
{
  // Compute positive value since we are computing a residual not a rhs
  return _thermal_conductivity[_qp] * _grad_test[_i][_qp](_component);
}
