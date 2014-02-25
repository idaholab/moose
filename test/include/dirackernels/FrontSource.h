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

#ifndef FRONTSOURCE_H
#define FRONTSOURCE_H

// Moose Includes
#include "DiracKernel.h"

#include "TrackDiracFront.h"

//Forward Declarations
class FrontSource;

template<>
InputParameters validParams<FrontSource>();

/**
 * An example showing how point sources can be applied based on
 * where a moving front is.  To do this we use a NodalUserObject to pick
 * the points where the sources should be applied.
 */
class FrontSource : public DiracKernel
{
public:
  FrontSource(const std::string & name, InputParameters parameters);
  virtual void addPoints();
  virtual Real computeQpResidual();

protected:
  const Real & _value;

  const TrackDiracFront & _front_tracker;
};

#endif //FRONTSOURCE_H
