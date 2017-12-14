/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
