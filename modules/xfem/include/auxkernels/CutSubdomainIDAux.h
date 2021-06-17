//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "GeometricCutUserObject.h"

class XFEM;

class CutSubdomainIDAux : public AuxKernel
{
public:
  static InputParameters validParams();

  CutSubdomainIDAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

private:
  /// Pointer to the GeometricCutUserObject
  const GeometricCutUserObject * _cut;
};
