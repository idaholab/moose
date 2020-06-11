#pragma once

#include "InitialCondition.h"

class PsbtIC;

template <>
InputParameters validParams<PsbtIC>();

/**
 * An abstract class for ICs relating to the PSBT fluid temperature benchmarks
 */
class PsbtIC : public InitialCondition
{
public:
  PsbtIC(const InputParameters & params);

protected:
  /**
   * Find the (row, column) indices of the subchannel containing a given point.
   */
  std::pair<int, int> index_point(const Point & p) const;
};
