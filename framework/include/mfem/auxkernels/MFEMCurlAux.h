#pragma once
#include "../common/pfem_extras.hpp"
#include "MFEMAuxKernel.h"

/*
Class to set an auxvariable to be the curl of a variable.
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
  // Name of auxvariable to store the result of the auxkernel in.
  VariableName _source_var_name;
  // Pointer to source gridfunction.
  mfem::ParGridFunction & _source_var;
  // FESpaces
  mfem::ParFiniteElementSpace & _hcurl_fespace;
  mfem::ParFiniteElementSpace & _hdiv_fespace;
  // Curl operator
  mfem::common::ParDiscreteCurlOperator _curl;
};
