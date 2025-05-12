//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "NodeElemConstraint.h"
#include "ADNodeElemConstraint.h"

template <bool is_ad>
class GenericNodeElemConstraint : public NodeElemConstraint
{
public:
  static InputParameters validParams() { return NodeElemConstraint::validParams(); };
  GenericNodeElemConstraint(const InputParameters & parameters) : NodeElemConstraint(parameters) {}
};

template <>
class GenericNodeElemConstraint<true> : public ADNodeElemConstraint
{
public:
  static InputParameters validParams() { return ADNodeElemConstraint::validParams(); };
  GenericNodeElemConstraint(const InputParameters & parameters) : ADNodeElemConstraint(parameters) {}
};
