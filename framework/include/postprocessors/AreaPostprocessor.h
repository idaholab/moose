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

#ifndef AREAPOSTPROCESSOR_H
#define AREAPOSTPROCESSOR_H

#include "SideIntegralPostprocessor.h"

//Forward Declarations
class AreaPostprocessor;

template<>
InputParameters validParams<AreaPostprocessor>();

/**
 * This postprocessor computes the area of a specified block.
 */
class AreaPostprocessor : public SideIntegralPostprocessor
{
public:
  AreaPostprocessor(const std::string & name, InputParameters parameters);
  virtual void threadJoin(const UserObject & y);

protected:
  virtual Real computeQpIntegral();
};

#endif
