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
#include "MFEMFESpace.h"

/**
 * Constructs and stores an mfem::ParFiniteElementSpace object. Access using the
 * getFESpace() accessor.
 */
class MFEMGenericFESpace : public MFEMFESpace
{
public:
  static InputParameters validParams();

  MFEMGenericFESpace(const InputParameters & parameters);

  virtual bool isScalar() const override;

  virtual bool isVector() const override;

protected:
  /// Get the name of the desired FECollection.
  virtual std::string getFECName() const override;

  /// Get the number of degrees of freedom per basis function needed
  /// in this finite element space.
  virtual int getVDim() const override;

private:
  /// The name of the finite element collection
  const std::string _fec_name;

  /// The number of degrees of freedom per basis function
  const int _vdim;
};

#endif
