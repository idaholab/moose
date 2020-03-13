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
 * This class constructs a functional expansion in cylindrical space using a 1D series for the axial
 * direction and a 2D disc series for (r, t).
 */
class CylindricalDuo final : public CompositeSeriesBasisInterface
{
public:
  CylindricalDuo(const std::string & who_is_using_me);
  CylindricalDuo(const std::vector<MooseEnum> & domain,
                 const std::vector<std::size_t> & order,
                 const std::vector<MooseEnum> & series_types,
                 const std::string & who_is_using_me,
                 MooseEnum expansion_type,
                 MooseEnum generation_type);

  // Virtual overrides
  virtual void setPhysicalBounds(const std::vector<Real> & bounds) final;
};
