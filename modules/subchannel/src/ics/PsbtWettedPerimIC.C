#include "PsbtWettedPerimIC.h"

registerMooseObject("MooseApp", PsbtWettedPerimIC);

template <>
InputParameters
validParams<PsbtWettedPerimIC>()
{
  return validParams<PsbtIC>();
}

PsbtWettedPerimIC::PsbtWettedPerimIC(const InputParameters & params)
  : PsbtIC(params)
{
}

Real
PsbtWettedPerimIC::value(const Point & p)
{
  // Define geometry parameters.
  constexpr Real pitch {0.0126};  // in m
  constexpr Real rod_diameter {0.00950};  // in m
  constexpr Real gap {0.00095};  // in m (extra width on periphery channels)
  constexpr Real rod_circumference {M_PI * rod_diameter};

  // Determine which channel this point is in and if that channel lies at an
  // edge or corner of the assembly.
  auto inds = index_point(p);
  auto i = inds.first;
  auto j = inds.second;
  bool is_corner = (i == 0 && j == 0) || (i == 5 && j == 0)
                   || (i == 0 && j == 5) || (i == 5 && j == 5);
  bool is_edge = (i == 0 || j == 0 || i == 5 || j == 5);

  // Compute and return the wetted perimeter.
  if (is_corner) {
    return 0.25*rod_circumference + pitch + 2*gap;
  } else if (is_edge) {
    return 0.5*rod_circumference + pitch;
  } else {
    return rod_circumference;
  }
}
