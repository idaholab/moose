//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SidePostprocessor.h"

/**
 * This postprocessor computes displacements normal to a provided
 * set of boundaries
 */
class NormalBoundaryDisplacement : public SidePostprocessor
{
public:
  static InputParameters validParams();

  NormalBoundaryDisplacement(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void finalize() override;
  virtual void execute() override;
  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  enum class NormalValType
  {
    AVERAGE,
    ABSOLUTE_AVERAGE,
    MAX,
    ABSOLUTE_MAX
  };

  /// the type of the normal displacement value
  NormalValType _value_type;

  /// number of components in _disp
  unsigned int _ncomp;

  /// displacement variable disp_x, disp_y, disp_z
  std::vector<const VariableValue *> _disp;

  /// boundary displacement value accumulated during execute
  Real _boundary_displacement;

  /// area of the boundary/boundaries
  Real _area;
};
