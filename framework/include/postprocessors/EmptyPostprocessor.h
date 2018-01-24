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

#ifndef EMPTYPOSTPROCESSOR_H
#define EMPTYPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

// Forward Declarations
class EmptyPostprocessor;

template <>
InputParameters validParams<EmptyPostprocessor>();

class EmptyPostprocessor : public GeneralPostprocessor
{
public:
  EmptyPostprocessor(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual Real getValue() override { return 0; }
};

#endif
