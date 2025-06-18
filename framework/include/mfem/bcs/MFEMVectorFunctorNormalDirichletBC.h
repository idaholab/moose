//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#pragma once

#include "MFEMVectorFunctorDirichletBCBase.h"

class MFEMVectorFunctorNormalDirichletBC : public MFEMVectorFunctorDirichletBCBase
{
public:
  static InputParameters validParams();
  MFEMVectorFunctorNormalDirichletBC(const InputParameters & parameters);
  ~MFEMVectorFunctorNormalDirichletBC() override = default;
  void ApplyBC(mfem::GridFunction & gridfunc) override;
};

#endif
