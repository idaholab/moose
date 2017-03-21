/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef DEFORMEDGRAINMATERIAL_H
#define DEFORMEDGRAINMATERIAL_H

#include "Material.h"

// Forward Declarations
class DeformedGrainMaterial;
class GrainTrackerInterface;

template <>
InputParameters validParams<DeformedGrainMaterial>();

/**
 * Calculates The Deformation Energy associated with a specific dislocation density.
 * The rest of parameters are the same as in the grain growth model
 */
class DeformedGrainMaterial : public Material
{
public:
  DeformedGrainMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// total number of grains
  const unsigned int _op_num;

  /// order parameter values
  std::vector<const VariableValue *> _vals;

  const Real _length_scale;
  const Real _int_width;
  const Real _time_scale;
  const Real _GBMobility;

  /// the GB Energy
  const Real _GBE;

  /// the average dislocation density
  const Real _Disloc_Den;

  /// the elastic modulus
  const Real _Elas_Mod;

  /// the Length of Burger's Vector
  const Real _Burg_vec;

  /// the same parameters that appear in the original grain growth model
  MaterialProperty<Real> & _kappa;
  MaterialProperty<Real> & _gamma;
  MaterialProperty<Real> & _L;
  MaterialProperty<Real> & _mu;
  MaterialProperty<Real> & _tgrad_corr_mult;

  /// the prefactor needed to calculate the deformation energy from dislocation density
  MaterialProperty<Real> & _beta;

  /// dislocation density in grain i
  MaterialProperty<Real> & _Disloc_Den_i;

  /// the average/effective dislocation density
  MaterialProperty<Real> & _rho_eff;

  /// the deformation energy
  MaterialProperty<Real> & _Def_Eng;

  // Constants

  /// number of deformed grains
  const unsigned int _deformed_grain_num;

  /// Grain tracker object
  const GrainTrackerInterface & _grain_tracker;
  const Real _kb;
  const Real _JtoeV;
};

#endif // DEFORMEDGRAINMATERIAL_H
