/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#include "PackedColumn.h"

template <>
InputParameters
validParams<PackedColumn>()
{
  InputParameters params = validParams<Material>();

  // Add a parameter to get the radius of the spheres in the column (used later to interpolate
  // permeability).
  params.addParam<Real>("sphere_radius",
                        "The radius of the steel spheres that are packed in the "
                        "column.  Used to interpolate _permeability.");

  return params;
}

PackedColumn::PackedColumn(const InputParameters & parameters)
  : Material(parameters),

    // Get the one parameter from the input file
    _sphere_radius(getParam<Real>("sphere_radius")),

    // Declare two material properties.  This returns references that we
    // hold onto as member variables
    _permeability(declareProperty<Real>("permeability")),
    _viscosity(declareProperty<Real>("viscosity"))
{
  // From the paper: Table 1
  std::vector<Real> sphere_sizes = {1, 3};
  std::vector<Real> permeability = {0.8451e-9, 8.968e-9};

  // Set the x,y data on the LinearInterpolation object.
  _permeability_interpolation.setData(sphere_sizes, permeability);

  // The _sphere_radius is a constant, so we can compute the
  // interpolated permeability once as well.
  _interpolated_permeability = _permeability_interpolation.sample(_sphere_radius);
}

void
PackedColumn::computeQpProperties()
{
  _viscosity[_qp] = 7.98e-4; // (Pa*s) Water at 30 degrees C (Wikipedia)
  _permeability[_qp] = _interpolated_permeability;
}
