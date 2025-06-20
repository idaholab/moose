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
#include "MFEMSubMesh.h"

/**
 * Constructs and stores an mfem::ParSubMesh object as
 * as a restriction of the parent mesh to the set of user-specified subdomains.
 * Access using the getSubMesh() accessor.
 */
class MFEMDomainSubMesh : public MFEMSubMesh, public MFEMBlockRestrictable
{
public:
  static InputParameters validParams();
  MFEMDomainSubMesh(const InputParameters & parameters);

protected:
  virtual void buildSubMesh() override;
};

#endif
