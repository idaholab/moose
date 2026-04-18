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

#include "MFEMExecutedObject.h"
#include "MFEMContainers.h"

namespace Moose::MFEM
{
/**
 * Class to construct an auxiliary solver used to update a complex auxvariable.
 */
class ComplexAuxKernel : public ExecutedObject
{
public:
  static InputParameters validParams();

  ComplexAuxKernel(const InputParameters & parameters);
  virtual ~ComplexAuxKernel() = default;

  /// Method called to update any owned objects upon an FE space update
  virtual void update() {};

  virtual std::optional<std::string> suppliedVariableName() const override;

  /// Method to add a scaled complex variable to another complex variable.
  void complexAdd(mfem::ParComplexGridFunction & a,
                  const mfem::ParComplexGridFunction & b,
                  const std::complex<mfem::real_t> scale = {1.0, 0.0});

  /// Method to scale a complex variable by a complex constant.
  void complexScale(mfem::ParComplexGridFunction & a,
                    const std::complex<mfem::real_t> scale = {1.0, 0.0});

protected:
  /// Name of complex auxvariable to store the result of the auxkernel in.
  const AuxVariableName _result_var_name;

  /// Reference to result complex gridfunction.
  mfem::ParComplexGridFunction & _result_var;

  /// Counter to keep track of FE space updates
  long _sequence{0};
};

} // namespace Moose::MFEM
#endif
