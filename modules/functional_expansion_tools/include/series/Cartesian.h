//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CompositeSeriesBasisInterface.h"

/**
 * This class constructs a functional expansion using a separate series for each Cartesian
 * dimension. 1D, 2D, and 3D domains are supported.
 */
class Cartesian final : public CompositeSeriesBasisInterface
{
public:
  Cartesian(const std::string & who_is_using_me);
  Cartesian(const std::vector<MooseEnum> & domain,
            const std::vector<std::size_t> & order,
            const std::vector<MooseEnum> & series_types,
            const std::string & who_is_using_me,
            MooseEnum expansion_type,
            MooseEnum generation_type);

  // Overrides from FunctionalBasisInterface
  virtual void setPhysicalBounds(const std::vector<Real> & bounds) final;
};
