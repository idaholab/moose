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

#include "MFEMSimplifiedFESpace.h"

/**
 * Constructs a scalar finite element space of NURBS basis functions, for isogeometric
 * analysis (IGA). Requires the mesh the space is defined on to be a NURBS mesh.
 */
class MFEMNURBSFESpace : public MFEMSimplifiedFESpace
{
public:
  static InputParameters validParams();

  MFEMNURBSFESpace(const InputParameters & parameters);

  virtual bool isScalar() const override { return true; }

  virtual bool isVector() const override { return false; }

protected:
  /// Get the name of the desired FECollection.
  virtual std::string getFECName() const override;

  /// Get the number of degrees of freedom per basis function needed
  /// in this finite element space.
  virtual int getVDim() const override;

  /// Constructs the fespace, degree elevating the NURBS extension of the mesh first if
  /// basis functions of higher order than the mesh geometry have been requested.
  virtual std::shared_ptr<mfem::ParFiniteElementSpace> buildFESpace() const override;
};

#endif
