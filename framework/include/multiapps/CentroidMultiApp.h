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
#ifndef CENTROIDMULTIAPP_H
#define CENTROIDMULTIAPP_H

#include "TransientMultiApp.h"
#include "BlockRestrictable.h"

class CentroidMultiApp;

template <>
InputParameters validParams<CentroidMultiApp>();

/**
 * Automatically generates Sub-App positions from centroids of elements in the master mesh.
 */
class CentroidMultiApp : public TransientMultiApp, public BlockRestrictable
{
public:
  CentroidMultiApp(const InputParameters & parameters);

protected:
  /**
   * fill in _positions with the positions of the sub-aps
   */
  virtual void fillPositions() override;
};

#endif // CENTROID_H
