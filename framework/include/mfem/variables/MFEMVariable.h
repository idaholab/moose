#pragma once

#include "MFEMFESpace.h"
#include "MFEMGeneralUserObject.h"

class MFEMVariable : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMVariable(const InputParameters & parameters);
  virtual ~MFEMVariable();

  const MFEMFESpace & fespace;
  unsigned int components;
};
