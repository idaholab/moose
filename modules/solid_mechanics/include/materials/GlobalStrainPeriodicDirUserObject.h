//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "UserObject.h"
#include "ElementUserObject.h"

/**
 * This class provides interface for extracting the periodic directions
 * from UserObjects associated with global strain calculation
 */
class GlobalStrainPeriodicDirUserObject : public ElementUserObject
{
public:
  static InputParameters validParams();

  GlobalStrainPeriodicDirUserObject(const InputParameters & parameters);

  virtual const VectorValue<bool> & getPeriodicDirections() const;

  void initialize() override;
  void execute() override;
  void threadJoin(const UserObject & uo) override;
  void finalize() override;

protected:
  const unsigned int _dim;

  const std::vector<const MooseVariableFieldBase *> _disp_var;

  VectorValue<bool> _periodic_dir;
};
