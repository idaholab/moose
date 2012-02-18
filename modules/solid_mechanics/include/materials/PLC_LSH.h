#ifndef PLC_LSH_H
#define PLC_LSH_H

#include "SolidModel.h"

// Forward declarations
class PLC_LSH;

template<>
InputParameters validParams<PLC_LSH>();

/**
 * Combined power-law creep and linear strain hardening material
 * Power law creep is specified by the time-hardening form
 * edot = A(sigma)**n * exp(-Q/(RT)) * t**m
 */

class PLC_LSH : public SolidModel
{
public:
  PLC_LSH( const std::string & name,
                 InputParameters parameters );

protected:

  Real _coefficient;
  Real _n_exponent;
  Real _m_exponent;
  Real _activation_energy;
  Real _gas_constant;
  
  Real _yield_stress;
  Real _hardening_constant;

  unsigned int _max_its;
  bool _output_iteration_info;
  Real _relative_tolerance;
  Real _absolute_tolerance;

  Real _stress_tolerance;

  MaterialProperty<SymmTensor> & _creep_strain;
  MaterialProperty<SymmTensor> & _creep_strain_old;

  MaterialProperty<SymmTensor> & _plastic_strain;
  MaterialProperty<SymmTensor> & _plastic_strain_old;

  MaterialProperty<Real> & _hardening_variable;
  MaterialProperty<Real> & _hardening_variable_old;

  MaterialProperty<Real> & _del_p;


  /// Compute the stress (sigma += deltaSigma)
  virtual void computeStress();

  void computeCreep(SymmTensor & creep_strain_increment,
                    SymmTensor & stress_new );
  void computeLSH( const SymmTensor & creep_strain_increment,
                   SymmTensor & plastic_strain_increment,
                   SymmTensor & stress_new );

private:

};

#endif //PLC_LSHMATERIAL_H
