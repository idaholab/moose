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

#ifndef GRAINTRACKERHALORM_H
#define GRAINTRACKERHALORM_H

#include "ElementSideNeighborLayers.h"

// Forward declarations
class GrainTrackerHaloRM;

template <>
InputParameters validParams<GrainTrackerHaloRM>();

/**
 * GrainTrackerHaloRM is used to ensure a level of elements at least as thick as the desired halo
 * level is available on each processor's partition to support detecting grain interaction.
 */
class GrainTrackerHaloRM : public ElementSideNeighborLayers
{
public:
  GrainTrackerHaloRM(const InputParameters & parameters);

  virtual std::string getInfo() const override;
};

#endif /* GRAINTRACKERHALORM_H */
