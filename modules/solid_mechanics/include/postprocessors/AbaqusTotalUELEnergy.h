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

class AbaqusUserElement;

/**
 * This postprocessor computes the sum of a UEL energy component over the mesh
 */
class AbaqusTotalUELEnergy : public ElementPostprocessor
{
public:
  static InputParameters validParams();

  AbaqusTotalUELEnergy(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject &) override;
  virtual void finalize() override;

  virtual Real getValue() const override;

protected:
  std::vector<const AbaqusUserElement *> _uel_uo;
  const std::size_t _component;

  Real _sum;
};
