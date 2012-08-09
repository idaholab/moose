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

#ifndef AVERAGENODALVARIABLEVALUE_H
#define AVERAGENODALVARIABLEVALUE_H

#include "NodalPostprocessor.h"

//Forward Declarations
class AverageNodalVariableValue;
class MooseMesh;

template<>
InputParameters validParams<AverageNodalVariableValue>();

class AverageNodalVariableValue : public NodalPostprocessor
{
public:
  AverageNodalVariableValue(const std::string & name, InputParameters parameters);

  virtual void initialize();
  virtual void execute();

  /**
   * This will return the degrees of freedom in the system.
   */
  virtual Real getValue();

  void threadJoin(const UserObject & y);

protected:
  Real _avg;
  unsigned int _n;
};

#endif //AVERAGENODALVARIABLEVALUE_H
