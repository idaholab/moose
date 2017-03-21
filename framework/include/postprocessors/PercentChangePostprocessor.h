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
#ifndef PERCENTCHANGEPOSTPROCESSOR_H
#define PERCENTCHANGEPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

class PercentChangePostprocessor;

template <>
InputParameters validParams<PercentChangePostprocessor>();

/**
 * This postprocessor displays the change in the postprocessor between
 * adjacent timesteps
 */

class PercentChangePostprocessor : public GeneralPostprocessor
{
public:
  PercentChangePostprocessor(const InputParameters & parameters);
  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;

protected:
  const PostprocessorValue &_postprocessor, &_postprocessor_old;
};

#endif /* PERCENTCHANGEPOSTPROCESSOR_H */
