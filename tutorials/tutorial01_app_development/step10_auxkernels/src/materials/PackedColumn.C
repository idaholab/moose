#include "PackedColumn.h"

registerMooseObject("BabblerApp", PackedColumn);

InputParameters
PackedColumn::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Computes the permeability of a porous medium made up of packed "
                             "steel spheres of a specified diameter in accordance with Pamuk and "
                             "Ozdemir (2012). This also provides a specified dynamic viscosity of "
                             "the fluid in the medium.");

  // Optional params for ball diameter and viscosity - inputs must satisfy range checked conditions
  params.addRangeCheckedParam<Real>(
      "diameter",
      1.0,
      "(1 <= diameter) & (diameter <= 3)",
      "The diameter (mm) of the steel spheres packed within the column that is used to compute "
      "its permeability. The input must be in the range [1, 3]. The default value is 1 mm.");
  params.addRangeCheckedParam<Real>(
      "viscosity",
      7.98e-04,
      "viscosity != 0",
      "The dynamic viscosity ($\\mu$) of the fluid, the default value is that of water at 30 "
      "degrees Celcius (7.98e-04 Pa-s).");

  return params;
}

PackedColumn::PackedColumn(const InputParameters & parameters)
  : Material(parameters),
    _diameter(getParam<Real>("diameter")),
    _input_viscosity(getParam<Real>("viscosity")),

    // Declare two material properties by getting a reference from the MOOSE Material system
    _permeability(declareADProperty<Real>("permeability")),
    _viscosity(declareADProperty<Real>("viscosity"))
{
  // From Pamuk and Ozdemir (2012): Table 1
  std::vector<Real> sphere_sizes = {1, 3};                // mm
  std::vector<Real> permeability = {0.8451e-9, 8.968e-9}; // m^2

  // Set the abscissa-ordinate data on the LinearInterpolation object.
  _permeability_interpolation.setData(sphere_sizes, permeability);
}

void
PackedColumn::computeQpProperties()
{
  _permeability[_qp] = _permeability_interpolation.sample(_diameter);
  _viscosity[_qp] = _input_viscosity;
}
