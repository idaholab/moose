//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
