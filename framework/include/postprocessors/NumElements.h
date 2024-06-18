//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

/**
 * Counts the number of elements in the entire mesh
 */
class NumElements : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  NumElements(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  virtual Real getValue() const override;

private:
  enum class ElemFilter
  {
    ACTIVE,
    TOTAL,
  };

  /// Whether to count all elements or only the active ones
  const ElemFilter _filt;

  /// Mesh to act on
  const MeshBase & _mesh;
};
