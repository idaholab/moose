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
  
  
  Real _tolerance;
  unsigned int _max_its;
  bool _output_iteration_info;
  
  //Real _shear_modulus;
  //Real _ebulk3;
  // Real _K;
  Real _yield_stress;
  Real _hardening_constant;
  
  


  MaterialProperty<RealTensorValue> & _plastic_strain;
  
  MaterialProperty<RealTensorValue> & _plastic_strain_old;

  MaterialProperty<Real> & _hardening_variable;  
  MaterialProperty<Real> & _hardening_variable_old;
  

  ColumnMajorMatrix _identity;

  
  
  
  /// Compute the stress (sigma += deltaSigma)
  virtual void computeStress();
  
private:

};

#endif //LINEARSTRAINHARDENINGMATERIAL_H
