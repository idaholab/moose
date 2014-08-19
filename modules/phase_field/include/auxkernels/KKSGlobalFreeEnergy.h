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

  MaterialProperty<Real> & _prop_fa;
  MaterialProperty<Real> & _prop_fb;
  MaterialProperty<Real> & _prop_h;
  MaterialProperty<Real> & _prop_g;

  Real _w;
};

#endif //KKSGLOBALFREEENERGY_H
