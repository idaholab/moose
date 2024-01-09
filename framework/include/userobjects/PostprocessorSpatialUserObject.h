//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

/**
 * User object that holds a postprocessor value
 *
 * This object can be used to associate a postprocessor value with a geometrical
 * location in a mutli app setup
 */
class PostprocessorSpatialUserObject : public GeneralUserObject
{
public:
  PostprocessorSpatialUserObject(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

  virtual Real spatialValue(const Point & /*p*/) const override;

protected:
  const Real & _value;

public:
  static InputParameters validParams();
};
