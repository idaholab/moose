#ifndef KKSXEVACSOLIDMATERIAL_H
#define KKSXEVACSOLIDMATERIAL_H

#include "DerivativeBaseMaterial.h"

// Forward Declarations
class KKSXeVacSolidMaterial;

template<>
InputParameters validParams<KKSXeVacSolidMaterial>();

class KKSXeVacSolidMaterial : public DerivativeBaseMaterial
{
public:
  KKSXeVacSolidMaterial(const std::string & name,
          InputParameters parameters);

protected:
  virtual unsigned int expectedNumArgs();

  virtual Real computeF();
  virtual Real computeDF(unsigned int arg);
  virtual Real computeD2F(unsigned int arg1, unsigned int arg2);

private:
  /// Temperature in [K]
  const Real _T;

  /// Atomic volume in [Ang^3]
  const Real _Omega;

  /// Bolzmann constant
  const Real _kB;

  /// Formation energy of a tri-vacancy in UO2
  const Real _Efv;

  /// Formation energy of a Xenon Atom in a tri-vacancy
  /// (TODO: if cmg>cmv consider interstitial Xe)
  const Real _Efg;

  // helper function to return a well defined c*log(c)
  Real cLogC(Real c);
};

#endif //KKSXEVACSOLIDMATERIAL_H
