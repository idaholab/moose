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
#include "MFEMAuxKernel.h"

/*
 * Class to scale and sum two MFEM variables, and store the result in a third variable.
 */
class MFEMSumAux : public MFEMAuxKernel
{
public:
  static InputParameters validParams();

  MFEMSumAux(const InputParameters & parameters);

  virtual ~MFEMSumAux() = default;

  // Computes the auxvariable.
  virtual void execute() override;

protected:
  // Names of input MFEMVariables to sum.
  const std::vector<VariableName> & _var_names;
  // Scalar factors to multiply the input variables by.
  const std::vector<mfem::real_t> _scale_factors;
  /// Pointers to input variable gridfunctions.
  std::vector<const mfem::ParGridFunction *> _summed_vars;
};

#endif
