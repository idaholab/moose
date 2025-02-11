#ifdef MFEM_ENABLED

#pragma once

#include "MFEMGeneralUserObject.h"

class MFEMScalarIC : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();
  MFEMScalarIC(const InputParameters & params);
  virtual void execute() override;
};

#endif
