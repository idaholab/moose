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

class MaterialCopyUserObject : public GeneralUserObject
{
public:
  static InputParameters validParams();

  MaterialCopyUserObject(const InputParameters & parameters);

  virtual ~MaterialCopyUserObject() {}

  /**
   * Called before execute() is ever called so that data can be cleared.
   */
  virtual void initialize() {}

  /**
   * Compute the hit positions for this timestep
   */
  virtual void execute();

  virtual void finalize() {}

protected:
  MooseMesh & _mesh;
  std::vector<Real> _copy_times;
  unsigned int _copy_from_element;
  unsigned int _copy_to_element;
  Real _time_tol;
};
