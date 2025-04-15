#ifdef MFEM_ENABLED

#pragma once
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/pfem_extras.hpp"
#include "libmesh/restore_warnings.h"
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
  const VariableName _source_var_name;
  // Reference to source gridfunction.
  const mfem::ParGridFunction & _source_var;
  // Scalar factor to multiply the result by.
  const mfem::real_t _scale_factor;
  // Divergence operator
  mfem::common::ParDiscreteDivOperator _div;
};

#endif
