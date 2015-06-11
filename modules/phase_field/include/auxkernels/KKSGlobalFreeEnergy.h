/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef KKSGLOBALFREEENERGY_H
#define KKSGLOBALFREEENERGY_H

#include "AuxKernel.h"
#include "Material.h"

//Forward Declarations
class KKSGlobalFreeEnergy;

template<>
InputParameters validParams<KKSGlobalFreeEnergy>();

/**
 * Compute the global free energy in the KKS Model
 * \f$ F = hF_a + (1-h)F_b \f$
 */
class KKSGlobalFreeEnergy : public AuxKernel
{
public:
  KKSGlobalFreeEnergy(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeValue();

  const MaterialProperty<Real> & _prop_fa;
  const MaterialProperty<Real> & _prop_fb;
  const MaterialProperty<Real> & _prop_h;
  const MaterialProperty<Real> & _prop_g;

  Real _w;
};

#endif //KKSGLOBALFREEENERGY_H
