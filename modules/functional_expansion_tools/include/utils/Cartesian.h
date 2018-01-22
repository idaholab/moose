// This file is part of the MOOSE framework
// https://www.mooseframework.org
//
// All rights reserved, see COPYRIGHT for full restrictions
// https://github.com/idaholab/moose/blob/master/COPYRIGHT
//
// Licensed under LGPL 2.1, please see LICENSE for details
// https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CARTESIAN_H
#define CARTESIAN_H

#include "CompositeSeriesBasisInterface.h"

class Cartesian final : public CompositeSeriesBasisInterface
{
public:
  Cartesian();
  Cartesian(const std::vector<MooseEnum> & domain,
            const std::vector<std::size_t> & order,
            const std::vector<MooseEnum> & series_types);

  // Overrides from FunctionalBasisInterface
  virtual void setPhysicalBounds(const std::vector<Real> & bounds) final;
};

#endif // CARTESIAN_H
