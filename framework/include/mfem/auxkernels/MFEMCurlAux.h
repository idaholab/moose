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
Class to set an H(div) auxvariable to be the curl of an H(curl) vector variable.
*/
class MFEMCurlAux : public MFEMAuxKernel
{
public:
  static InputParameters validParams();

  MFEMCurlAux(const InputParameters & parameters);

  virtual ~MFEMCurlAux() = default;

  // Computes the auxvariable.
  virtual void execute() override;

protected:
  // Name of source MFEMVariable to take the curl of.
  const VariableName _source_var_name;
  // Reference to source gridfunction.
  const mfem::ParGridFunction & _source_var;
  // Scalar factor to multiply the result by.
  const mfem::real_t _scale_factor;
  // Curl operator
  mfem::common::ParDiscreteCurlOperator _curl;
};

#endif
