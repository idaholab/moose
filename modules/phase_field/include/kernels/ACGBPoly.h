//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "ACBulk.h"

// Forward Declarations

class ACGBPoly : public ACBulk<Real>
{
public:
  static InputParameters validParams();

  ACGBPoly(const InputParameters & parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const VariableValue & _c;
  unsigned int _c_var;

  const MaterialProperty<Real> & _mu;
  const MaterialProperty<Real> & _gamma;

  Real _en_ratio;
};
