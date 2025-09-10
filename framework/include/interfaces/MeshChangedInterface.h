//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

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

  MeshChangedInterface(const InputParameters & params);

#ifdef MOOSE_KOKKOS_ENABLED
  /**
   * Special constructor used for Kokkos functor copy during parallel dispatch
   */
  MeshChangedInterface(const MeshChangedInterface & object, Moose::Kokkos::FunctorCopy);
#endif

  virtual ~MeshChangedInterface() = default;

  /**
   * Called on this object when the mesh changes
   */
  virtual void meshChanged() {}

protected:
  /// Reference to FEProblemBase instance
  FEProblemBase & _mci_feproblem;
};
