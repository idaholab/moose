//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

#include "RichardsVarNames.h"
#include "RichardsDensity.h"
#include "RichardsRelPerm.h"
#include "RichardsSeff.h"
#include "RichardsSat.h"
#include "RichardsSUPG.h"

// Forward Declarations

class RichardsMaterial : public Material
{
public:
  static InputParameters validParams();

  RichardsMaterial(const InputParameters & parameters);

protected:
  /// porosity as entered by the user
  Real _material_por;

  /// porosity changes.  if not entered they default to zero
  const VariableValue & _por_change;
  const VariableValue & _por_change_old;

  /// permeability as entered by the user
  RealTensorValue _material_perm;

  /// gravity as entered by user
  RealVectorValue _material_gravity;

  /// material properties
  MaterialProperty<Real> & _porosity_old;
  MaterialProperty<Real> & _porosity;
  MaterialProperty<RealTensorValue> & _permeability;
  MaterialProperty<RealVectorValue> & _gravity;

  /// The variable names userobject for the Richards variables
  const RichardsVarNames & _richards_name_UO;
  unsigned int _num_p;

  std::vector<const RichardsRelPerm *> _material_relperm_UO;
  std::vector<const RichardsSeff *> _material_seff_UO;
  std::vector<const RichardsSat *> _material_sat_UO;
  std::vector<const RichardsDensity *> _material_density_UO;
  std::vector<const RichardsSUPG *> _material_SUPG_UO;

  const std::vector<const VariableValue *> _perm_change;

  virtual void computeProperties();

  /**
   * computes the quadpoint values of the porepressure(s)
   * and effective saturation(s), and their derivatives
   * wrt the variables in the system.
   * These are then used in computeProperties to compute
   * relative permeability, density, and so on
   */
  void computePandSeff();

  /**
   * Computes the "derived" quantities --- those that
   * depend on porepressure(s) and effective saturation(s) ---
   * such as density, relative permeability, mass, flux, etc
   * @param qp The quadpoint to evaluate the quantites at
   */
  void computeDerivedQuantities(unsigned int qp);

  /**
   * Computes 2nd derivatives of the flux.
   * These are needed by kernels if doing SUPG
   * @param qp The quadpoint to evaluate the quantites at
   */
  void compute2ndDerivedQuantities(unsigned int qp);

  /**
   * Assigns and zeroes the MaterialProperties
   * associated with SUPG.
   * @param qp The quadpoint to assign+zero at
   */
  void zeroSUPG(unsigned int qp);

  /// Computes the tauvel_SUPG and its derivatives
  void computeSUPG();

private:
  /// trace of permeability tensor
  Real _trace_perm;

  std::vector<Real> _material_viscosity;

  /// old values of porepressure(s)
  MaterialProperty<std::vector<Real>> & _pp_old;

  /// porepressure(s)
  MaterialProperty<std::vector<Real>> & _pp;

  /// d(porepressure_i)/d(variable_j)
  MaterialProperty<std::vector<std::vector<Real>>> & _dpp_dv;

  /// d^2(porepressure_i)/d(variable_j)/d(variable_k)
  MaterialProperty<std::vector<std::vector<std::vector<Real>>>> & _d2pp_dv;

  /// fluid viscosity (or viscosities in the multiphase case)
  MaterialProperty<std::vector<Real>> & _viscosity;

  /// old fluid density (or densities for multiphase problems)
  MaterialProperty<std::vector<Real>> & _density_old;

  /// fluid density (or densities for multiphase problems)
  MaterialProperty<std::vector<Real>> & _density;

  /// d(density_i)/d(variable_j)
  MaterialProperty<std::vector<std::vector<Real>>> & _ddensity_dv;

  /// old effective saturation
  MaterialProperty<std::vector<Real>> & _seff_old;

  /// effective saturation (vector of effective saturations in case of multiphase)
  MaterialProperty<std::vector<Real>> & _seff; // effective saturation

  /// d(Seff_i)/d(variable_j)
  MaterialProperty<std::vector<std::vector<Real>>> & _dseff_dv; // d(seff)/dp

  /// d^2(Seff_i)/d(variable_j)/d(variable_k)
  MaterialProperty<std::vector<std::vector<std::vector<Real>>>> & _d2seff_dv;

  /// old saturation
  MaterialProperty<std::vector<Real>> & _sat_old;

  /// saturation (vector of saturations in case of multiphase)
  MaterialProperty<std::vector<Real>> & _sat;

  /// d(saturation_i)/d(variable_j)
  MaterialProperty<std::vector<std::vector<Real>>> & _dsat_dv;

  /// relative permeability (vector of relative permeabilities in case of multiphase)
  MaterialProperty<std::vector<Real>> & _rel_perm;

  /// d(relperm_i)/d(variable_j)
  MaterialProperty<std::vector<std::vector<Real>>> & _drel_perm_dv;

  /// old value of fluid mass (a vector of masses for multicomponent)
  MaterialProperty<std::vector<Real>> & _mass_old;

  /// fluid mass (a vector of masses for multicomponent)
  MaterialProperty<std::vector<Real>> & _mass;

  /// d(fluid mass_i)/dP_j (a vector of masses for multicomponent)
  MaterialProperty<std::vector<std::vector<Real>>> & _dmass;

  /// permeability*(grad(P) - density*gravity)  (a vector of these for multicomponent)
  MaterialProperty<std::vector<RealVectorValue>> & _flux_no_mob;

  /// d(_flux_no_mob_i)/d(variable_j)
  MaterialProperty<std::vector<std::vector<RealVectorValue>>> & _dflux_no_mob_dv;

  /// d(_flux_no_mob_i)/d(grad(variable_j))
  MaterialProperty<std::vector<std::vector<RealTensorValue>>> & _dflux_no_mob_dgradv;

  /// fluid flux (a vector of fluxes for multicomponent)
  MaterialProperty<std::vector<RealVectorValue>> & _flux;

  /// d(Richards flux_i)/d(variable_j), here flux_i is the i_th flux, which is itself a RealVectorValue
  MaterialProperty<std::vector<std::vector<RealVectorValue>>> & _dflux_dv;

  /// d(Richards flux_i)/d(grad(variable_j)), here flux_i is the i_th flux, which is itself a RealVectorValue
  MaterialProperty<std::vector<std::vector<RealTensorValue>>> & _dflux_dgradv;

  /// d^2(Richards flux_i)/d(variable_j)/d(variable_k), here flux_i is the i_th flux, which is itself a RealVectorValue
  MaterialProperty<std::vector<std::vector<std::vector<RealVectorValue>>>> & _d2flux_dvdv;

  /// d^2(Richards flux_i)/d(grad(variable_j))/d(variable_k), here flux_i is the i_th flux, which is itself a RealVectorValue
  MaterialProperty<std::vector<std::vector<std::vector<RealTensorValue>>>> & _d2flux_dgradvdv;

  /// d^2(Richards flux_i)/d(variable_j)/d(grad(variable_k)), here flux_i is the i_th flux, which is itself a RealVectorValue.  We should have _d2flux_dvdgradv[i][j][k] = _d2flux_dgradvdv[i][k][j], but i think it is more clear having both, and hopefully not a blowout on memory/CPU.
  MaterialProperty<std::vector<std::vector<std::vector<RealTensorValue>>>> & _d2flux_dvdgradv;

  MaterialProperty<std::vector<RealVectorValue>> & _tauvel_SUPG; // tauSUPG * velSUPG
  MaterialProperty<std::vector<std::vector<RealTensorValue>>> &
      _dtauvel_SUPG_dgradp; // d (_tauvel_SUPG_i)/d(_grad_variable_j)
  MaterialProperty<std::vector<std::vector<RealVectorValue>>> &
      _dtauvel_SUPG_dp; // d (_tauvel_SUPG_i)/d(variable_j)

  /// d^2(density)/dp_j/dP_k - used in various derivative calculations
  std::vector<std::vector<std::vector<Real>>> _d2density;

  /// d^2(relperm_i)/dP_j/dP_k - used in various derivative calculations
  std::vector<std::vector<std::vector<Real>>> _d2rel_perm_dv;

  std::vector<const VariableValue *> _pressure_vals;
  std::vector<const VariableValue *> _pressure_old_vals;
  std::vector<const VariableGradient *> _grad_p;

  /**
   * Zeroes 2nd derivatives of the flux.
   * The values are only calculated in compute2ndDerivedQuantites
   * if the SUPG UserObject says they are need.  This is for
   * computational efficiency as these are very expensive to calculate
   * @param qp The quadpoint to evaluate the quantites at
   */
  void zero2ndDerivedQuantities(unsigned int qp);
};
