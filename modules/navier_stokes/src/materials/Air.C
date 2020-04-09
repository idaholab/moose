//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Air.h"

#include "libmesh/quadrature.h"

registerMooseObject("NavierStokesApp", Air);

InputParameters
Air::validParams()
{
  InputParameters params = NavierStokesMaterial::validParams();

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
