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

#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"
#include "MFEMGeneralUserObject.h"
#include "MFEMBlockRestrictable.h"
#include "CoefficientManager.h"

class MFEMFunctorMaterial : public MFEMGeneralUserObject, public MFEMBlockRestrictable
{
public:
  static InputParameters validParams();
  static libMesh::Point pointFromMFEMVector(const mfem::Vector & vec);

  MFEMFunctorMaterial(const InputParameters & parameters);
  virtual ~MFEMFunctorMaterial();

protected:
  Moose::MFEM::CoefficientManager & _properties;
};

#endif
