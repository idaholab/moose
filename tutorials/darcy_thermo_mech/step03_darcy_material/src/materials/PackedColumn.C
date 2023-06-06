//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PackedColumn.h"
#include "Function.h"

registerMooseObject("DarcyThermoMechApp", PackedColumn);

InputParameters
PackedColumn::validParams()
{
  InputParameters params = Material::validParams();

  // Parameter for radius of the spheres used to interpolate permeability.
  params.addParam<FunctionName>("radius",
                                "1.0",
                                "The radius of the steel spheres (mm) that are packed in the "
                                "column for computing permeability.");
  params.addParam<Real>(
      "viscosity",
      7.98e-4,
      "The viscosity ($\\mu$) of the fluid in Pa, the default is for water at 30 degrees C.");

  return params;
}

PackedColumn::PackedColumn(const InputParameters & parameters)
  : Material(parameters),

    // Get the parameters from the input file
    _radius(getFunction("radius")),
    _input_viscosity(getParam<Real>("viscosity")),

    // Declare two material properties by getting a reference from the MOOSE Material system
    _permeability(declareADProperty<Real>("permeability")),
    _viscosity(declareADProperty<Real>("viscosity"))
{
}

void
PackedColumn::computeQpProperties()
{
  // From the paper: Table 1
  std::vector<Real> sphere_sizes = {1, 3};
  std::vector<Real> permeability = {0.8451e-9, 8.968e-9};

  Real value = _radius.value(_t, _q_point[_qp]);
  mooseAssert(value >= 1 && value <= 3,
              "The radius range must be in the range [1, 3], but " << value << " provided.");

  _viscosity[_qp] = _input_viscosity;

  // We'll calculate permeability using a simple linear interpolation of the two points above:
  //          y0 * (x1 - x) + y1 * (x - x0)
  //  y(x) = -------------------------------
  //                     x1 - x0
  _permeability[_qp] =
      (permeability[0] * (sphere_sizes[1] - value) + permeability[1] * (value - sphere_sizes[0])) /
      (sphere_sizes[1] - sphere_sizes[0]);
}
