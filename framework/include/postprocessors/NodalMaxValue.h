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

#ifndef NODALMAXVALUE_H
#define NODALMAXVALUE_H

#include "NodalVariablePostprocessor.h"

// Forward Declarations
class NodalMaxValue;

template <>
InputParameters validParams<NodalMaxValue>();

/**
 * This class computes a maximum (over all the nodal values) of the
 * coupled variable.
 */
class NodalMaxValue : public NodalVariablePostprocessor
{
public:
  NodalMaxValue(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  Real _value;
};

#endif
