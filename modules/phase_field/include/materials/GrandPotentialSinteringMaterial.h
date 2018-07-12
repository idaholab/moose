/****************************************************************/
/*                  DO NOT MODIFY THIS HEADER                   */
/*                           Marmot                             */
/*                                                              */
/*            (c) 2017 Battelle Energy Alliance, LLC            */
/*                     ALL RIGHTS RESERVED                      */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*             Under Contract No. DE-AC07-05ID14517             */
/*             With the U. S. Department of Energy              */
/*                                                              */
/*             See COPYRIGHT for full restrictions              */
/****************************************************************/

#ifndef GRANDPOTENTIALSINTERINGMATERIAL
#define GRANDPOTENTIALSINTERINGMATERIAL

#include "Material.h"
#include "DerivativeMaterialInterface.h"

/**
 * This material calculates necessary parameters for the grand potential sintering model.
 * Especially those related to switching functions and thermodynamics.
 **/

class GrandPotentialSinteringMaterial;

template <>
InputParameters validParams<GrandPotentialSinteringMaterial>();

class GrandPotentialSinteringMaterial : public DerivativeMaterialInterface<Material>
{
public:
  GrandPotentialSinteringMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  const unsigned int _neta;                // number of solid phase order paramters
  std::vector<const VariableValue *> _eta; // solid phase order parameters
  std::vector<VariableName> _eta_name;
  const VariableValue & _w; // chemical potential
  const NonlinearVariableName _w_name;
  const VariableValue & _phi; // void phase order parameter
  const NonlinearVariableName _phi_name;
  const VariableValue & _T;           // temperature
  const MaterialProperty<Real> & _kv; // void energy coefficient
  const MaterialProperty<Real> & _ks; // solid energy coefficient

  MaterialProperty<Real> & _hv; // void phase switching function
  MaterialProperty<Real> & _dhv;
  MaterialProperty<Real> & _d2hv;
  MaterialProperty<Real> & _hs; // solid phase switching function
  MaterialProperty<Real> & _dhs;
  MaterialProperty<Real> & _d2hs;
  MaterialProperty<Real> & _chi; // susceptibility
  MaterialProperty<Real> & _dchi;
  MaterialProperty<Real> & _d2chi;
  MaterialProperty<Real> & _rhov; // void phase vacancy density
  MaterialProperty<Real> & _drhovdw;
  MaterialProperty<Real> & _rhos; // solid phase vacancy density
  MaterialProperty<Real> & _drhosdw;
  std::vector<MaterialProperty<Real> *> _drhos;
  std::vector<std::vector<MaterialProperty<Real> *>> _d2rhos;
  MaterialProperty<Real> & _omegav; // void phase potential density
  MaterialProperty<Real> & _domegavdw;
  MaterialProperty<Real> & _d2omegavdw2;
  MaterialProperty<Real> & _omegas; // solid phase potential density
  MaterialProperty<Real> & _domegasdw;
  MaterialProperty<Real> & _d2omegasdw2;
  std::vector<MaterialProperty<Real> *> _domegasdeta;
  std::vector<MaterialProperty<Real> *> _d2omegasdwdeta;
  std::vector<std::vector<MaterialProperty<Real> *>> _d2omegasdetadeta;
  MaterialProperty<Real> & _mu; // energy barrier coefficient
  MaterialProperty<Real> & _dmu;
  MaterialProperty<Real> & _d2mu;
  MaterialProperty<Real> & _kappa; // gradient energy coefficient
  MaterialProperty<Real> & _dkappa;
  MaterialProperty<Real> & _d2kappa;
  MaterialProperty<Real> & _gamma; // interface profile coefficient

  const Real _sigma_s;   // surface energy
  const Real _sigma_gb;  // grain boundary energy
  const Real _int_width; // interface width
  const Real _switch;    // Parameter to determine accuracy of surface/GB phase switching function
  const Real _Ef;        // Vacancy formation energy
  const Real _c_gb;      // Extra grain boundary vacancy concentration
  const Real _Va;        // Atomic volume of species
  const Real _prefactor; // Vacancy concentration prefactor
  const Real _mu_s;      // mu value on surfaces
  const Real _mu_gb;     // mu value on grain boundaries
  const Real _kappa_s;   // kappa value on surfaces
  const Real _kappa_gb;  // kappa value on grain boundaries
  const Real _kB;        // Boltzmann constant
};

#endif // GRANDPOTENTIALSINTERINGMATERIAL
