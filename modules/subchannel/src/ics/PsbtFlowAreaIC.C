#include "PsbtFlowAreaIC.h"

registerMooseObject("MooseApp", PsbtFlowAreaIC);

template <>
InputParameters
validParams<PsbtFlowAreaIC>()
{
  return validParams<PsbtIC>();
}

PsbtFlowAreaIC::PsbtFlowAreaIC(const InputParameters & params)
  : PsbtIC(params)
{
}

Real
PsbtFlowAreaIC::value(const Point & p)
{
  // Define geometry parameters.
  constexpr Real pitch {0.0126};  // in m
  constexpr Real rod_diameter {0.00950};  // in m
  constexpr Real gap {0.00095};  // in m (extra width on periphery channels)

  // Compute the flow area for a standard channel (one that touches 4 pins).
  constexpr Real rod_area {0.25 * M_PI * rod_diameter * rod_diameter};
  constexpr Real standard_area {pitch*pitch - rod_area};

  // Determine which channel this point is in and if that channel lies at an
  // edge or corner of the assembly.
  auto inds = index_point(p);
  auto i = inds.first;
  auto j = inds.second;
  bool is_corner = (i == 0 && j == 0) || (i == 5 && j == 0)
                   || (i == 0 && j == 5) || (i == 5 && j == 5);
  bool is_edge = (i == 0 || j == 0 || i == 5 || j == 5);

  // Compute and return the channel area.
  if (is_corner) {
    return standard_area*0.25 + pitch*gap + gap*gap;
  } else if (is_edge) {
    return standard_area*0.5 + pitch*gap;
  } else {
    return standard_area;
  }
}
