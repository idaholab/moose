//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PFCTRADMATERIAL_H
#define PFCTRADMATERIAL_H

#include "Material.h"

// Forward Declarations
class PFCTradMaterial;

template <>
InputParameters validParams<PFCTradMaterial>();

class PFCTradMaterial : public Material
{
public:
  PFCTradMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

private:
  const unsigned int _order;

  MaterialProperty<Real> & _M;
  MaterialProperty<Real> & _a;
  MaterialProperty<Real> & _b;

  MaterialProperty<Real> & _C0;
  MaterialProperty<Real> & _C2;
  MaterialProperty<Real> & _C4;
  MaterialProperty<Real> & _C6;
  MaterialProperty<Real> & _C8;
};

#endif // PFCTRADMATERIAL_H
