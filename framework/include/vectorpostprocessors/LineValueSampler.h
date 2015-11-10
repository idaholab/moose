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

#ifndef LINEVALUESAMPLER_H
#define LINEVALUESAMPLER_H

// MOOSE includes
#include "PointSamplerBase.h"

// Forward Declarations
class LineValueSampler;

template<>
InputParameters validParams<LineValueSampler>();

class LineValueSampler : public PointSamplerBase
{
public:
  LineValueSampler(const InputParameters & parameters);

  virtual ~LineValueSampler() {}
};

#endif
