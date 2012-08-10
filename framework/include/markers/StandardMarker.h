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

#ifndef STANDARDMARKER_H
#define STANDARDMARKER_H

#include "Marker.h"

class StandardMarker;

template<>
InputParameters validParams<StandardMarker>();

class StandardMarker : public Marker
{
public:
  StandardMarker(const std::string & name, InputParameters parameters);
  virtual ~StandardMarker(){};

protected:
  virtual int computeElementMarker();
};

#endif /* STANDARDMARKER_H */
