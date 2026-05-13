//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMAuxKernel.h"

class MFEMNDtoRTAux : public MFEMAuxKernel
{
public:
  static InputParameters validParams();

  MFEMNDtoRTAux(const InputParameters & parameters);

  virtual void update() override;

protected:
  const AuxVariableName _nd_source_var_name;
  const mfem::ParGridFunction & _nd_source_var;
  const mfem::real_t _sign;
};

#endif
