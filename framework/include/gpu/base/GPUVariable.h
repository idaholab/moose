//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUTypes.h"

#include "MooseTypes.h"
#include "MooseVariableBase.h"

class Coupleable;

class GPUVariable
{
  friend class Coupleable;

private:
  // Whether this variable was coupled
  bool _coupled = false;
  // The number of components
  unsigned int _components = 1;
  // The vector tag ID
  TagID _tag = Moose::INVALID_TAG_ID;
  // Variable number of each component
  GPUArray<unsigned int> _var;
  // System number of each component
  GPUArray<unsigned int> _sys;
  // Default value of each component
  GPUArray<Real> _default_value;

public:
  GPUVariable() {}
  GPUVariable(const MooseVariableBase & variable, const TagID tag) { init(variable, tag); }
  GPUVariable(const MooseVariableBase & variable, const TagName & tag_name = Moose::SOLUTION_TAG)
  {
    init(variable, tag_name);
  }
  // Initialize this GPU variable from a MOOSE variable
  void init(const MooseVariableBase & variable, const TagName & tag_name = Moose::SOLUTION_TAG);
  void init(const MooseVariableBase & variable, const TagID tag);

public:
  KOKKOS_FUNCTION bool coupled() const { return _coupled; }
  KOKKOS_FUNCTION unsigned int components() { return _components; }
  KOKKOS_FUNCTION TagID tag() const { return _tag; }
  KOKKOS_FUNCTION unsigned int var(unsigned int comp = 0) const { return _var[comp]; }
  KOKKOS_FUNCTION unsigned int sys(unsigned int comp = 0) const { return _sys[comp]; }
  KOKKOS_FUNCTION Real value(unsigned int comp = 0) const { return _default_value[comp]; }
};
