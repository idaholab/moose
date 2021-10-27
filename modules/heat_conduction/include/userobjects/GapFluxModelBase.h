//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceUserObject.h"

/**
 * Base class for gap flux models used by ModularGapConductanceConstraint
 */
class GapFluxModelBase : public InterfaceUserObject
{
public:
  static InputParameters validParams();

  GapFluxModelBase(const InputParameters & parameters);

  virtual ADReal computeFlux(const ADReal & gap_width, unsigned int qp) const = 0;

  virtual void finalize() final{};
  virtual void threadJoin(const UserObject &) final{};
};
