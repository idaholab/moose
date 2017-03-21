/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
