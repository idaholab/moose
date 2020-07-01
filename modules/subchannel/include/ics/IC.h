#pragma once

#include "InitialCondition.h"

/**
 * An abstract class for ICs relating to the PSBT fluid temperature benchmarks
 */
class IC : public InitialCondition
{
public:
  IC(const InputParameters & params);

protected:
  /**
   * Find the (row, column) indices of the subchannel containing a given point.
   */
  std::pair<unsigned int, unsigned int> index_point(const Point & p) const;

public:
  static InputParameters validParams();
};
