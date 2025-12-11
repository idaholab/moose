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

/*
 * Class to scale and sum an arbitrary number of MFEM complex variables into an auxiliary variable.
 */
class MFEMComplexSumAux : public MFEMComplexAuxKernel
{
public:
  static InputParameters validParams();

  MFEMComplexSumAux(const InputParameters & parameters);

  virtual ~MFEMComplexSumAux() = default;

  // Computes the auxvariable.
  virtual void execute() override;

protected:
  // Names of input MFEMVariables to sum.
  const std::vector<VariableName> & _var_names;
  // Real scalar factors to multiply the input variables by.
  const std::vector<mfem::real_t> _scale_factors_real;
  // Imaginary scalar factors to multiply the input variables by.
  const std::vector<mfem::real_t> _scale_factors_imag;
  /// Pointers to input variable gridfunctions.
  std::vector<const mfem::ParComplexGridFunction *> _summed_vars;
};

#endif
