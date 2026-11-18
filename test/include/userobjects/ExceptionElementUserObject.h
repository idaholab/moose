//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"

/**
 * Element user object that throws a std::runtime_error from execute() on a single rank
 */
class ExceptionElementUserObject : public ElementUserObject
{
public:
  static InputParameters validParams();

  ExceptionElementUserObject(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void threadJoin(const UserObject &) override {}
  virtual void finalize() override;

protected:
  /// The rank on which to throw from execute
  const processor_id_type _rank;
};
