/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "HeatConductionTimeDerivative.h"

template<>
InputParameters validParams<HeatConductionTimeDerivative>()
{
  InputParameters params = validParams<TimeDerivative>();
  // Density may be changing with deformation, so we must integrate
  // over current volume by setting the use_displaced_mesh flag.
  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<bool>("use_heat_capacity",
                        false,
                        "Use a single material property, 'heat_capacity', "
                        "as the coefficient of the time derivative.");

  params.addParam<MaterialPropertyName>("specific_heat_name",
                                        "specific_heat",
                                        "Property name of the specific heat material property (Default: specific_heat)");

  params.addParam<MaterialPropertyName>("heat_capacity_name",
                                        "heat_capacity",
                                        "Property name of the heat capacity material property, this is only used when "
                                        "'use_specific_heat' is true (Default: heat_capacity)");

  params.addParam<MaterialPropertyName>("density_name",
                                        "density",
                                        "Property name of the density material property (Default: density)");
  return params;
}


HeatConductionTimeDerivative::HeatConductionTimeDerivative(const InputParameters & parameters) :
    TimeDerivative(parameters),
    _use_heat_capacity(getParam<bool>("use_heat_capacity")),
    _specific_heat(NULL),
    _density(NULL)
{
  // Use the Heat Capacity based formulation
  if (_use_heat_capacity)
  {
    _specific_heat = &getMaterialProperty<Real>("heat_capacity_name");
    _density = &_one;
  }

  // Use the density and specific heat based formulation
  else
  {
    _specific_heat = &getMaterialProperty<Real>("specific_heat_name");
    _density = &getMaterialProperty<Real>("density_name");
  }
}

void
HeatConductionTimeDerivative::initialSetup()
{
  // Initialize the unity material property
  _one.resize(_fe_problem.getMaxQps());
  for (unsigned int i = 0; i < _one.size(); ++i)
    _one[i] = 1;
}

Real
HeatConductionTimeDerivative::computeQpResidual()
{
  return (*_specific_heat)[_qp] * (*_density)[_qp] * TimeDerivative::computeQpResidual();
}

Real
HeatConductionTimeDerivative::computeQpJacobian()
{
  return (*_specific_heat)[_qp] * (*_density)[_qp] * TimeDerivative::computeQpJacobian();
}
