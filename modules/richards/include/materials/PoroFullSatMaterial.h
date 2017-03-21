/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROFULLSATMATERIAL_H
#define POROFULLSATMATERIAL_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"

// Forward Declarations
class PoroFullSatMaterial;

template <>
InputParameters validParams<PoroFullSatMaterial>();

/**
 * Material designed to calculate and store all the
 * quantities needed for the fluid-flow part of
 * poromechanics, assuming a fully-saturated, single-phase
 * fluid with constant bulk modulus
 */
class PoroFullSatMaterial : public DerivativeMaterialInterface<Material>
{
public:
  PoroFullSatMaterial(const InputParameters & parameters);

protected:
  /// porosity at zero porepressure and volumetric strain
  Real _phi0;

  /// Biot coefficient
  Real _alpha;

  /// 1/K, where K is the solid bulk modulus.  Usually 1/K = C_iijj, where C is the compliance matrix: strain_ij = C_ijkl stress_kl
  Real _one_over_K;

  /// 1/Kf, where Kf is the fluid bulk modulus.
  Real _one_over_Kf;

  /// whether to use constant porosity (set _porosity = _phi0 always)
  bool _constant_porosity;

  /// porepressure variable
  const VariableValue & _porepressure;

  /// name given by user to the porepressure variable
  std::string _porepressure_name;

  /// number of displacement variables supplied
  unsigned int _ndisp;

  /// grad(displacement)
  std::vector<const VariableGradient *> _grad_disp;

  /// volumetric strain = strain_ii
  MaterialProperty<Real> & _vol_strain;

  /// old value of volumetric strain
  MaterialProperty<Real> & _vol_strain_old;

  /// Biot coefficient
  MaterialProperty<Real> & _biot_coefficient;

  /// porosity
  MaterialProperty<Real> & _porosity;

  /// d(porosity)/d(porepressure)
  MaterialProperty<Real> & _dporosity_dP;

  /// d(porosity)/d(volumetric_strain)
  MaterialProperty<Real> & _dporosity_dep;

  /// 1/M, where M is the Biot modulus
  MaterialProperty<Real> & _one_over_biot_modulus;

  /// d(1/M)/d(porepressure)
  MaterialProperty<Real> & _done_over_biot_modulus_dP;

  /// d(1/M)/d(volumetric_strain)
  MaterialProperty<Real> & _done_over_biot_modulus_dep;

  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();
};

#endif // POROFULLSATMATERIAL_H
