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

#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"
#include "MFEMGeneralUserObject.h"
#include "CoefficientManager.h"

class MFEMFunctorMaterial : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();
  std::vector<std::string> subdomainsToStrings(const std::vector<SubdomainName> & blocks);
  static libMesh::Point pointFromMFEMVector(const mfem::Vector & vec);

  MFEMFunctorMaterial(const InputParameters & parameters);
  virtual ~MFEMFunctorMaterial();

  const std::vector<SubdomainName> & getBlocks() const { return _block_ids; }

protected:
  const std::vector<SubdomainName> & _block_ids;
  Moose::MFEM::CoefficientManager & _properties;
};

#endif
