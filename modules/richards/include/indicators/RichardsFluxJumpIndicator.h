/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSFLUXJUMPINDICATOR_H
#define RICHARDSFLUXJUMPINDICATOR_H

#include "JumpIndicator.h"
#include "RichardsPorepressureNames.h"

class RichardsFluxJumpIndicator;

template<>
InputParameters validParams<RichardsFluxJumpIndicator>();

/**
 * Jump of the Richards mass-flux*viscosity
 * = density * relative_permeability * permeability * (grad(porepressure) - density * gravity)
 */
class RichardsFluxJumpIndicator :
  public JumpIndicator
{
public:
  RichardsFluxJumpIndicator(const std::string & name, InputParameters parameters);
  virtual ~RichardsFluxJumpIndicator(){};

protected:

  virtual Real computeQpIntegral();

  /**
   * holds info regarding the names of the porepressure variables
   * and methods for extracting values of these variables
   */
  const RichardsPorepressureNames & _pp_name_UO;

  /**
   * the index of this variable in the list of porepressure variables
   * held by _pp_name_UO.  Eg
   * if porepressure_vars = 'pwater pgas poil' in the _pp_name_UO
   * and this kernel has variable = pgas, then _pvar = 1
   * This is used to index correctly into seff_UO, sat_UO, density_UO, etc.
   */
  unsigned int _pvar;

  /// fluid density (vector of densities if multiphase)
  MaterialProperty<std::vector<Real> > &_density;

  /// fluid relative permeability (vector of relperms if multiphase)
  MaterialProperty<std::vector<Real> > &_rel_perm;

  /// gravity vector pointing in downwards direction
  MaterialProperty<RealVectorValue> &_gravity;

  /// material permeability
  MaterialProperty<RealTensorValue> & _permeability;

  /// neighboring value of fluid density (vector of densities if multiphase)
  MaterialProperty<std::vector<Real> > &_density_n;

  /// neighboring value of fluid relative permeability (vector of relperms if multiphase)
  MaterialProperty<std::vector<Real> > &_rel_perm_n;

  /// neighboring value of gravity vector pointing in downwards direction
  MaterialProperty<RealVectorValue> &_gravity_n;

  /// neighboring value of material permeability
  MaterialProperty<RealTensorValue> & _permeability_n;
};

#endif /* RICHARDSFLUXJUMPINDICATOR_H */
