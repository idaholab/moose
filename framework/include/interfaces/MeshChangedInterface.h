//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Forward declarations
class FEProblemBase;
class InputParameters;

/**
 * Interface for notifications that the mesh has changed.
 */
class MeshChangedInterface
{
public:
  static InputParameters validParams();

  MeshChangedInterface(const InputParameters & params, bool initialize = true);

  virtual ~MeshChangedInterface() = default;

  /**
   * Called on this object when the mesh changes
   */
  virtual void meshChanged() {}

protected:
  /// Reference to FEProblemBase instance
  FEProblemBase & _mci_feproblem;
};
