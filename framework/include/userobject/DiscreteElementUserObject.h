//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "ElementUserObject.h"

class DiscreteElementUserObject : public ElementUserObject
{
public:
  static InputParameters validParams();

  DiscreteElementUserObject(const InputParameters & parameters);

  virtual void initialize() override;

  /// @{ Block all methods that are not used in explicitly called UOs
  virtual void execute() override final;
  virtual void finalize() override final;
  virtual void threadJoin(const UserObject &) override final;
  /// @}

  bool needThreadedCopy() const override final { return true; }
};
