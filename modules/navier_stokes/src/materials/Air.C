/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "Air.h"

// libMesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<Air>()
{
  InputParameters params = validParams<NavierStokesMaterial>();

  params.addClassDescription("Air.");
  // Allow the user to specify the dynamic viscosity from the input file,
  // otherwise use the value at 300K by default
  params.addParam<Real>("dynamic_viscosity", 1.9830e-5, "in kg/m/s");

  return params;
}

Air::Air(const InputParameters & parameters)
  : NavierStokesMaterial(parameters), _mu(getParam<Real>("dynamic_viscosity"))
{
}

void
Air::computeProperties()
{
  // Constant "\mu" values (kg / m / s) for air at various temperatures.  Pick the
  // one that makes sense for the current calculation.  A class employing
  // Sutherland's curve fit for air might also be good to have...
  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
  {
    // Return the default dynamic_viscosity or whatever was set in the input file.
    _dynamic_viscosity[qp] = _mu;
  }

  // Call base class's computeProperties() function
  NavierStokesMaterial::computeProperties();
}
