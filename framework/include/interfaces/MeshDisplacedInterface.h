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
 * Interface for objects acting when the mesh has been displaced
 */
class MeshDisplacedInterface
{
public:
  static InputParameters validParams();

  MeshDisplacedInterface(const InputParameters & params);
  virtual ~MeshDisplacedInterface() = default;

  /**
   * Called on this object when the displaced mesh gets updated
   */
  virtual void meshDisplaced() {}

protected:
  /// Reference to FEProblemBase instance
  FEProblemBase & _mdi_feproblem;
};
