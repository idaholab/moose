//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"
#include "SubChannelMesh.h"

/**
 * Calculates the value of a user selected variable at a user selected point in the assembly
 */
class SubChannelPointValue : public GeneralPostprocessor
{
public:
  static InputParameters validParams();
  SubChannelPointValue(const InputParameters & params);
  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override;
  virtual Real getValue() const override;

protected:
  /// geometric information
  const SubChannelMesh & _mesh;
  /// axial location [m]
  const Real & _height;
  /// subchannel index
  const unsigned int & _i_ch;
  /// name of variable
  const VariableName & _variable;
  /// whether the variable is first-order Lagrange, so it can be interpolated between nodes
  bool _nodal;
  /// subchannel centroid at the requested height, for variables that are not nodal
  Point _point;
  /// axial cell containing the height
  unsigned int _iz;
  /// linear interpolation weight of the height within the axial cell
  Real _weight;
  /// value we want to calculate
  Real _value;
};
