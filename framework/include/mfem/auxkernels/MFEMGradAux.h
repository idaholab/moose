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

namespace Moose::MFEM
{
/**
 * Class to set an H(curl) auxvariable to be the gradient of a H1 scalar variable.
 */
class GradAux : public AuxKernel
{
public:
  static InputParameters validParams();

  GradAux(const InputParameters & parameters);

  virtual ~GradAux() = default;

  /// Computes the auxvariable.
  virtual void execute() override;

  // Updates grad operator.
  virtual void update() override;

protected:
  /// Name of source Moose::MFEM::Variable to take the gradient of.
  const VariableName _source_var_name;
  /// Reference to source gridfunction.
  const mfem::ParGridFunction & _source_var;
  /// Scalar factor to multiply the result by.
  const mfem::real_t _scale_factor;
  /// Grad operator
  mfem::common::ParDiscreteGradOperator _grad;
};

} // namespace Moose::MFEM
#endif
