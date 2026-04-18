//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMFESpace.h"

/// Class with common parameters for Moose::MFEM::VectorFESpace and
/// Moose::MFEM::ScalarFESpace.
namespace Moose::MFEM
{
class SimplifiedFESpace : public FESpace
{
public:
  static InputParameters validParams();

  SimplifiedFESpace(const InputParameters & parameters);

protected:
  /// Order of the basis functions in the finite element collection
  const int _fec_order;

  /// Returns the dimension of the problem (i.e., the highest dimension of the
  /// reference elements in the mesh).
  int getProblemDim() const;
};

} // namespace Moose::MFEM
#endif
