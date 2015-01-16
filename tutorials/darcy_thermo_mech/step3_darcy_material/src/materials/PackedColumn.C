#include "PackedColumn.h"

template<>
InputParameters validParams<PackedColumn>()
{
  InputParameters params = validParams<Material>();

  // Add a parameter to get the radius of the balls in the column (used later to interpolate permeability).
  params.addParam<Real>("ball_radius", "The radius of the steel balls that are packed in the column.  Used to interpolate _permeability.");

  return params;
}


PackedColumn::PackedColumn(const std::string & name, InputParameters parameters) :
    Material(name, parameters),

    // Get the one parameter from the input file
    _ball_radius(getParam<Real>("ball_radius")),

    // Declare two material properties.  This returns references that we
    // hold onto as member variables
    _permeability(declareProperty<Real>("permeability")),
    _viscosity(declareProperty<Real>("viscosity"))
{
  // Sigh: Still can't depend on C++11....
  std::vector<Real> ball_sizes(2);
  ball_sizes[0] = 1;
  ball_sizes[1] = 3;

  // From the paper: Table 1
  std::vector<Real> permeability(2);
  permeability[0] = 0.8451e-9;
  permeability[1] = 8.968e-9;

  // Set the x,y data on the LinearInterpolation object.
  _permeability_interpolation.setData(ball_sizes, permeability);
}

void
PackedColumn::computeQpProperties()
{
  _viscosity[_qp] = 7.98e-4; // (Pa*s) Water at 30 degrees C (Wikipedia)

  // Sample the LinearInterpolation object to get the permeability for the ball size
  _permeability[_qp] = _permeability_interpolation.sample(_ball_radius);
}
