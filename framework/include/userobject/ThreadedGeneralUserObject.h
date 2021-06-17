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

/// An instance of this object type has one copy per thread that runs on each thread.
class ThreadedGeneralUserObject : public GeneralUserObject
{
public:
  static InputParameters validParams();

  ThreadedGeneralUserObject(const InputParameters & parameters);
  virtual ~ThreadedGeneralUserObject() = default;
  virtual void threadJoin(const UserObject &) override;
  virtual void subdomainSetup() override{};

  bool needThreadedCopy() const override final { return true; }
};
