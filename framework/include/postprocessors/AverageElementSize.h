//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementPostprocessor.h"

/**
 * This postprocessor computes an average element size (h) for the whole domain.
 */
class AverageElementSize : public ElementPostprocessor
{
public:
  static InputParameters validParams();

  AverageElementSize(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

  virtual void finalize() override;
  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  Real _total_size;
  dof_id_type _elems;
};
