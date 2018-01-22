// This file is part of the MOOSE framework
// https://www.mooseframework.org
//
// All rights reserved, see COPYRIGHT for full restrictions
// https://github.com/idaholab/moose/blob/master/COPYRIGHT
//
// Licensed under LGPL 2.1, please see LICENSE for details
// https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CYLINDRICALDUO_H
#define CYLINDRICALDUO_H

#include "CompositeSeriesBasisInterface.h"

class CylindricalDuo final : public CompositeSeriesBasisInterface
{
public:
  CylindricalDuo();
  CylindricalDuo(const std::vector<MooseEnum> & domain,
                 const std::vector<std::size_t> & order,
                 const std::vector<MooseEnum> & series_types);
  virtual ~CylindricalDuo() = default;

  // Virtual overrides
  virtual void setPhysicalBounds(const std::vector<Real> & bounds) final;
};

#endif // CYLINDRICALDUO_H
