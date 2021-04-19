//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementSubdomainModifier.h"
#include "XFEM.h"

/**
 * CutElementSubdomainModifier switches the element subdomain ID based on the CutSubdomainID
 * marked by geometric cut userobjects.
 */
class CutElementSubdomainModifier : public ElementSubdomainModifier
{
public:
  static InputParameters validParams();

  CutElementSubdomainModifier(const InputParameters & parameters);

protected:
  virtual SubdomainID computeSubdomainID() override;

private:
  /// Pointer to the GeometricCutUserObject
  const GeometricCutUserObject * _cut;
};
