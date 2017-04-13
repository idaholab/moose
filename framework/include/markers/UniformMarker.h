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

#ifndef UNIFORMMARKER_H
#define UNIFORMMARKER_H

#include "Marker.h"

class UniformMarker;

template <>
InputParameters validParams<UniformMarker>();

class UniformMarker : public Marker
{
public:
  UniformMarker(const InputParameters & parameters);

protected:
  virtual MarkerValue computeElementMarker() override;

  MarkerValue _mark;
};

#endif /* UNIFORMMARKER_H */
