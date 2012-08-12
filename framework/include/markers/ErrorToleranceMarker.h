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

#ifndef ERRORTOLERANCEMARKER_H
#define ERRORTOLERANCEMARKER_H

#include "IndicatorMarker.h"

class ErrorToleranceMarker;

template<>
InputParameters validParams<ErrorToleranceMarker>();

class ErrorToleranceMarker : public IndicatorMarker
{
public:
  ErrorToleranceMarker(const std::string & name, InputParameters parameters);
  virtual ~ErrorToleranceMarker(){};

protected:
  virtual int computeElementMarker();

  Real _coarsen;
  Real _refine;
};

#endif /* ERRORTOLERANCEMARKER_H */
