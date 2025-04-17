//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FieldSplitPreconditioner.h"
#include "MooseStaticCondensationPreconditioner.h"

/**
 * Implements a field split preconditioner for a statically condensed system
 */
class StaticCondensationFieldSplitPreconditioner
  : public FieldSplitPreconditionerTempl<MooseStaticCondensationPreconditioner>
{
public:
  /**
   *  Constructor. Initializes SplitBasedPreconditioner data structures
   */
  static InputParameters validParams();

  StaticCondensationFieldSplitPreconditioner(const InputParameters & parameters);

  virtual void setupDM() override;
  virtual KSP getKSP() override;

protected:
  virtual const libMesh::DofMapBase & dofMap() const override;
  virtual const libMesh::System & system() const override;
  virtual std::string petscPrefix() const override;
};
