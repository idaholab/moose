//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"
#include "MultiBoundingBoxIC.h"

// Forward Declarations
class NestedBoundingBoxIC;

template <>
InputParameters validParams<NestedBoundingBoxIC>();

class NestedBoundingBoxIC : public MultiBoundingBoxIC
{
public:
  NestedBoundingBoxIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  /// value of interfacial width
  const Real _int_width;
};
