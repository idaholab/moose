#pragma once
#include "../common/pfem_extras.hpp"
#include "MFEMAuxKernel.h"

/*
Class to set an L2 auxvariable to be the divergence of a H(div) vector variable.
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
  // Pointer to source gridfunction.
  mfem::ParGridFunction & _source_var;
  // FESpaces
  mfem::ParFiniteElementSpace & _hdiv_fespace;
  mfem::ParFiniteElementSpace & _l2_fespace;
  // Divergence operator
  mfem::common::ParDiscreteCurlOperator _div;
};
