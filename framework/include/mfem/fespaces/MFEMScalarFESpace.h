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

class MFEMScalarFESpace : public MFEMSimplifiedFESpace
{
public:
  static InputParameters validParams();

  MFEMScalarFESpace(const InputParameters & parameters);

  virtual bool isScalar() const override { return true; }

  virtual bool isVector() const override { return false; }

protected:
  /// Get the name of the desired FECollection.
  virtual std::string getFECName() const override;

  /// Get the number of degrees of freedom per basis function needed
  /// in this finite element space.
  virtual int getVDim() const override;

private:
  /// Name of the family of finite element collections to use
  const std::string _fec_type;
  /// Name of the map types VALUE OR INTEGRAL to use (meaningfull only for L2)
  const std::string _fec_map;
};

#endif
