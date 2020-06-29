#include "PsbtIC.h"

InputParameters
PsbtIC::validParams()
{
  return InitialCondition::validParams();
}

PsbtIC::PsbtIC(const InputParameters & params) : InitialCondition(params) {}

std::pair<unsigned int, unsigned int>
PsbtIC::index_point(const Point & p) const
{
  constexpr Real pitch{0.0126}; // in m
  unsigned int i = (p(0) + 0.5 * pitch) / pitch;
  unsigned int j = (p(1) + 0.5 * pitch) / pitch;
  return {i, j};
}
