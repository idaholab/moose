//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "InterfaceUserObject.h"

/**
 * Interface user object with no material dependencies for selective material reinit testing.
 */
class MaterialReinitInterfaceUserObject : public InterfaceUserObject
{
public:
  static InputParameters validParams();

  MaterialReinitInterfaceUserObject(const InputParameters & parameters);

protected:
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & uo) override;
};
