#ifndef LINEARSTRAINHARDENING_H
#define LINEARSTRAINHARDENING_H

#include "MaterialModel.h"

// Forward declarations
class LinearStrainHardening;

template<>
InputParameters validParams<LinearStrainHardening>();

/**
 * Power-law creep material
 * edot = A(sigma)**n * exp(-Q/(RT))
 */

class LinearStrainHardening : public MaterialModel
{
public:
  LinearStrainHardening( const std::string & name,
                         InputParameters parameters );

protected:


  const Real _tolerance;
  const unsigned int _max_its;

  const Real _yield_stress;
  const Real _hardening_constant;

  MaterialProperty<SymmTensor> & _plastic_strain;
  MaterialProperty<SymmTensor> & _plastic_strain_old;

  MaterialProperty<Real> & _hardening_variable;
  MaterialProperty<Real> & _hardening_variable_old;

  /// Compute the stress (sigma += deltaSigma)
  virtual void computeStress();

private:

};

#endif //LINEARSTRAINHARDENING_H
