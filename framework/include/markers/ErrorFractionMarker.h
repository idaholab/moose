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

#ifndef ERRORFRACTIONMARKER_H
#define ERRORFRACTIONMARKER_H

#include "IndicatorMarker.h"

class ErrorFractionMarker;

template<>
InputParameters validParams<ErrorFractionMarker>();

class ErrorFractionMarker : public IndicatorMarker
{
public:
  ErrorFractionMarker(const std::string & name, InputParameters parameters);
  virtual ~ErrorFractionMarker(){};

  virtual void markerSetup();

protected:
  virtual int computeElementMarker();

  Real _coarsen;
  Real _refine;

  Real _max;
  Real _min;
  Real _delta;
  Real _refine_cutoff;
  Real _coarsen_cutoff;
};

#endif /* ERRORFRACTIONMARKER_H */
