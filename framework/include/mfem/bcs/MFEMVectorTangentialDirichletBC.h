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

#include "MFEMVectorDirichletBCBase.h"

namespace Moose::MFEM
{
class VectorTangentialDirichletBC : public VectorDirichletBCBase
{
public:
  static InputParameters validParams();
  VectorTangentialDirichletBC(const InputParameters & parameters);
  ~VectorTangentialDirichletBC() override = default;
  void ApplyBC(mfem::GridFunction & gridfunc) override;
};

} // namespace Moose::MFEM
#endif
