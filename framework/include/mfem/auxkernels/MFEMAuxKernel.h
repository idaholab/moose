#ifdef MFEM_ENABLED

#pragma once

#include "MFEMGeneralUserObject.h"
#include "MFEMContainers.h"

/*
Class to construct an auxiliary solver used to update an auxvariable.
*/
class MFEMAuxKernel : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMAuxKernel(const InputParameters & parameters);
  virtual ~MFEMAuxKernel() = default;

  // Method called to update any owned objects upon a mesh update.
  virtual void update(){};

protected:
  // Name of auxvariable to store the result of the auxkernel in.
  AuxVariableName _result_var_name;

  /// Reference to result gridfunction.
  mfem::ParGridFunction & _result_var;
};

#endif
