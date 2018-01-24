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

#ifndef CHANGEOVERTIMESTEPPOSTPROCESSOR_H
#define CHANGEOVERTIMESTEPPOSTPROCESSOR_H

#include "ChangeOverTimePostprocessor.h"

class ChangeOverTimestepPostprocessor;

template <>
InputParameters validParams<ChangeOverTimestepPostprocessor>();

/**
 * Computes the change in a post-processor value, or the magnitude of its
 * relative change, over a time step or over the entire transient.
 */
class ChangeOverTimestepPostprocessor : public ChangeOverTimePostprocessor
{
public:
  ChangeOverTimestepPostprocessor(const InputParameters & parameters);
};

#endif /* CHANGEOVERTIMESTEPPOSTPROCESSOR_H */
