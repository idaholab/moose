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

#ifndef INDICATORMARKER_H
#define INDICATORMARKER_H

#include "Marker.h"

class IndicatorMarker;

template<>
InputParameters validParams<IndicatorMarker>();

class IndicatorMarker : public Marker
{
public:
  IndicatorMarker(const std::string & name, InputParameters parameters);
  virtual ~IndicatorMarker(){};

protected:
  ErrorVector & _error_vector;
};

#endif /* INDICATORMARKER_H */
