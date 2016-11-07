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
#ifndef DIFFERENCEPOSTPROCESSOR_H
#define DIFFERENCEPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

class DifferencePostprocessor;

template <>
InputParameters validParams<DifferencePostprocessor>();

/**
 * Computes the difference between two postprocessors
 *
 * result = value1 - value2
 */
class DifferencePostprocessor : public GeneralPostprocessor
{
public:
  DifferencePostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual PostprocessorValue getValue() override;
  virtual void threadJoin(const UserObject & uo) override;

protected:
  const PostprocessorValue & _value1;
  const PostprocessorValue & _value2;
};

#endif /* DIFFERENCEPOSTPROCESSOR_H */
