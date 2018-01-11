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
