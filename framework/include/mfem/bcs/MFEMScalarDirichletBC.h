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

#include "MFEMEssentialBC.h"

namespace Moose::MFEM
{
class ScalarDirichletBC : public EssentialBC
{
public:
  static InputParameters validParams();

  ScalarDirichletBC(const InputParameters & parameters);

  void ApplyBC(mfem::GridFunction & gridfunc) override;

protected:
  mfem::Coefficient & _coef;
};

} // namespace Moose::MFEM
#endif
