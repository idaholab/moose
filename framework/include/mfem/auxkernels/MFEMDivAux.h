#pragma once
#include "mfem/miniapps/common/pfem_extras.hpp"
#include "MFEMAuxKernel.h"

/*
Class to set an L2 auxvariable to be the divergence of an H(div) vector variable.
*/
class MFEMDivAux : public MFEMAuxKernel
{
public:
  static InputParameters validParams();

  MFEMDivAux(const InputParameters & parameters);

  virtual ~MFEMDivAux() = default;

  // Computes the auxvariable.
  virtual void execute() override;

protected:
  // Name of source MFEMVariable to take the divergence of.
  VariableName _source_var_name;
  // Reference to source gridfunction.
  mfem::ParGridFunction & _source_var;
  // Scalar factor to multiply the result by.
  mfem::real_t _scale_factor;
  // Divergence operator
  mfem::common::ParDiscreteDivOperator _div;
};
