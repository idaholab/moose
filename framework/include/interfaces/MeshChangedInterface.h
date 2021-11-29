//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseEnum.h"

// Forward declarations
class FEProblemBase;
class InputParameters;
template <typename T>
InputParameters validParams();

/**
 * Interface for notifications that the mesh has changed.
 */
class MeshChangedInterface
{
public:
  static InputParameters validParams();

  MeshChangedInterface(const InputParameters & params);
  virtual ~MeshChangedInterface() = default;

  /**
   * Called on this object when the mesh changes
   */
  virtual void meshChanged() {}

protected:
  /// Reference to FEProblemBase instance
  FEProblemBase & _mci_feproblem;
};
