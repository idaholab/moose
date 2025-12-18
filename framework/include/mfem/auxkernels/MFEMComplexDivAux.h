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

#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/pfem_extras.hpp"
#include "libmesh/restore_warnings.h"
#include "MFEMComplexAuxKernel.h"

/**
 * Class to set an L2 auxvariable to be the divergence of an H(div) vector variable.
 */
class MFEMComplexDivAux : public MFEMComplexAuxKernel
{
public:
  static InputParameters validParams();

  MFEMComplexDivAux(const InputParameters & parameters);

  virtual ~MFEMComplexDivAux() = default;

  /// Computes the auxvariable.
  virtual void execute() override;

protected:
  /// Name of source MFEMComplexVariable to take the divergence of.
  const VariableName _source_var_name;
  /// Reference to source gridfunction.
  const mfem::ParComplexGridFunction & _source_var;
  /// Scalar factor to multiply the result by.
  const std::complex<mfem::real_t> _scale_factor;
  /// Divergence operator
  mfem::common::ParDiscreteDivOperator _div;
};

#endif
