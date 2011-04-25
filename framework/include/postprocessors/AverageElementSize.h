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

#ifndef AVERAGEELEMENTSIZE_H
#define AVERAGEELEMENTSIZE_H

#include "ElementAverageValue.h"

//Forward Declarations
class AverageElementSize;

template<>
InputParameters validParams<AverageElementSize>();

/**
 * This postprocessor computes an average element size (h) for the whole domain.
 */
class AverageElementSize : public ElementAverageValue
{
public:
  AverageElementSize(const std::string & name, InputParameters parameters);

  virtual void initialize();
  virtual void execute();

  virtual Real computeIntegral();

  virtual Real getValue();
  virtual void threadJoin(const Postprocessor & y);

protected:
  int _elems;
};
 
#endif // AVERAGEELEMENTSIZE_H
