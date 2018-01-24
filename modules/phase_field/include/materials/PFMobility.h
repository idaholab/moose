//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PFMOBILITY_H
#define PFMOBILITY_H

#include "Material.h"

// Forward Declarations
class PFMobility;

template <>
InputParameters validParams<PFMobility>();

class PFMobility : public Material
{
public:
  PFMobility(const InputParameters & parameters);

protected:
  virtual void computeProperties();

private:
  MaterialProperty<Real> & _M;
  MaterialProperty<RealGradient> & _grad_M;
  MaterialProperty<Real> & _kappa_c;

  Real _mob;
  Real _kappa;
};

#endif // PFMOBILITY_H
