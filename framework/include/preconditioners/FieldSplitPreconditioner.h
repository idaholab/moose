//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MoosePreconditioner.h"

// Forward declarations
class NonlinearSystemBase;
class InputParameters;
namespace libMesh
{
class DofMapBase;
}

#include <vector>
#include <string>

/**
 * Base interface for field split preconditioner
 */
class FieldSplitPreconditionerBase
{
public:
  FieldSplitPreconditionerBase() = default;

  /**
   * setup the data management data structure that manages the field split
   */
  virtual void setupDM() = 0;

  /**
   * @returns The KSP object associated with the field split preconditioner
   */
  virtual KSP getKSP() = 0;
};

/**
 * Implements a preconditioner designed to map onto PETSc's PCFieldSplit.
 */
template <typename Base>
class FieldSplitPreconditionerTempl : public FieldSplitPreconditionerBase, public Base
{
public:
  /**
   *  Constructor. Initializes SplitBasedPreconditioner data structures
   */
  static InputParameters validParams();

  FieldSplitPreconditionerTempl(const InputParameters & parameters);

protected:
  /**
   * @returns The degree of freedom map to use for decomposition
   */
  virtual const libMesh::DofMapBase & dofMap() const = 0;

  /**
   * @returns The libMesh system
   */
  virtual const libMesh::System & system() const = 0;

  /**
   * @returns The prefix to pass to PETSc for the DM
   */
  virtual std::string prefix() const = 0;

  /**
   * creates the MOOSE data management object
   */
  void createMooseDM(DM * dm);

  /// The nonlinear system this FSP is associated with (convenience reference)
  NonlinearSystemBase & _nl;

  /**
   * The decomposition split
   */
  std::string _decomposition_split;
};

class FieldSplitPreconditioner : public FieldSplitPreconditionerTempl<MoosePreconditioner>
{
public:
  static InputParameters validParams();

  FieldSplitPreconditioner(const InputParameters & parameters);

  virtual void setupDM() override;
  virtual KSP getKSP() override;

protected:
  virtual const libMesh::DofMapBase & dofMap() const override;
  virtual const libMesh::System & system() const override;
  virtual std::string prefix() const override;
};
