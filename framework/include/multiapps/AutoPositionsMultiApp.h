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
#ifndef AUTOPOSITIONSMULTIAPP_H
#define AUTOPOSITIONSMULTIAPP_H

#include "TransientMultiApp.h"
#include "BoundaryRestrictable.h"

class AutoPositionsMultiApp;

template <>
InputParameters validParams<AutoPositionsMultiApp>();

/**
 * Automatically generates Sub-App positions from positions in the master app's mesh.
 */
class AutoPositionsMultiApp : public TransientMultiApp, public BoundaryRestrictable
{
public:
  AutoPositionsMultiApp(const InputParameters & parameters);

protected:
  /**
   * _must_ fill in _positions with the positions of the sub-aps
   */
  virtual void fillPositions() override;
};

#endif // AUTOPOSITIONSMULTIAPP_H
