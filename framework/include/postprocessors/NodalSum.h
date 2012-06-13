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

#ifndef NODALSUM_H
#define NODALSUM_H

#include "NodalPostprocessor.h"

//Forward Declarations
class NodalSum;
class MooseMesh;

template<>
InputParameters validParams<NodalSum>();

class NodalSum : public NodalPostprocessor
{
public:
  NodalSum(const std::string & name, InputParameters parameters);

  virtual void initialize();
  virtual void execute();

  /**
   * This will return the degrees of freedom in the system.
   */
  virtual Real getValue();

  void threadJoin(const Postprocessor & y);

protected:
  Real _sum;
};

#endif //NODALSUM_H
