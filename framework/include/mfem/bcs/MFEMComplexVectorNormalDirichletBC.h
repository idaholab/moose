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

#include "MFEMComplexVectorDirichletBCBase.h"

class MFEMComplexVectorNormalDirichletBC : public MFEMComplexVectorDirichletBCBase
{
public:
  static InputParameters validParams();
  MFEMComplexVectorNormalDirichletBC(const InputParameters & parameters);
  ~MFEMComplexVectorNormalDirichletBC() override = default;
  void ApplyBC(mfem::ParComplexGridFunction & gridfunc) override;
};

#endif
