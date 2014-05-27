/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSFLUX
#define RICHARDSFLUX

#include "Kernel.h"
#include "RichardsVarNames.h"

// Forward Declarations
class RichardsFlux;

template<>
InputParameters validParams<RichardsFlux>();

/**
 * Kernel = grad(permeability*relativepermeability/viscosity * (grad(pressure) - density*gravity))
 * This is mass flow according to the Richards equation
 */
class RichardsFlux : public Kernel
{
public:

  RichardsFlux(const std::string & name,
                        InputParameters parameters);


protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /**
   * holds info regarding the names of the Richards variables
   * and methods for extracting values of these variables
   */
  const RichardsVarNames & _richards_name_UO;

  /**
   * the index of this variable in the list of Richards variables
   * held by _richards_name_UO.  Eg
   * if richards_vars = 'pwater pgas poil' in the _richards_name_UO
   * and this kernel has variable = pgas, then _pvar = 1
   * This is used to index correctly into _viscosity, _seff, etc
   */
  unsigned int _pvar;

  /// fluid viscosity, or for multiphase a vector of viscosities
  MaterialProperty<std::vector<Real> > &_viscosity;

  /// gravity vector pointing in downwards direction
  MaterialProperty<RealVectorValue> &_gravity;

  /// material permeability
  MaterialProperty<RealTensorValue> & _permeability;

  /// effective saturation(s)
  MaterialProperty<std::vector<Real> > &_seff;

  /// derivative of effective saturation(s) wrt porepressure(s)
  MaterialProperty<std::vector<std::vector<Real> > > &_dseff;

  /// second derivative of effective saturation(s) wrt porepressure(s)
  MaterialProperty<std::vector<std::vector<std::vector<Real> > > > &_d2seff;

  /// fluid relative permeability (vector of relperms if multiphase)
  MaterialProperty<std::vector<Real> > &_rel_perm;

  /// derivative of relative permeability wrt fluid effective saturation
  MaterialProperty<std::vector<Real> > &_drel_perm;

  /// second derivative of relative permeability wrt fluid effective saturation
  MaterialProperty<std::vector<Real> > &_d2rel_perm;

  /// fluid density (vector of densities if multiphase)
  MaterialProperty<std::vector<Real> > &_density;

  /// derivative of fluid density wrt fluid porepressure
  MaterialProperty<std::vector<Real> > &_ddensity;

  /// second derivative of fluid density wrt fluid porepressure
  MaterialProperty<std::vector<Real> > &_d2density;

  /// grad_i grad_j porepressure.  This is used in SUPG
  VariableSecond & _second_u;

  /// interpolation function for the _second_u
  VariablePhiSecond & _second_phi;

  /// SUPGtau*SUPGvel (a vector of these if multiphase)
  MaterialProperty<std::vector<RealVectorValue> >&_tauvel_SUPG;

  /// derivative of SUPGtau*SUPGvel wrt grad(porepressure)
  MaterialProperty<std::vector<RealTensorValue> >&_dtauvel_SUPG_dgradp;

  /// derivative of SUPGtau*SUPGvel wrt porepressure
  MaterialProperty<std::vector<RealVectorValue> >&_dtauvel_SUPG_dp;

 private:

  /// mobility = density*relative-permeability
  Real mobility(Real density, Real relperm);

  /// derivative of mobility wrt porepressure
  Real dmobility_dp(Real density, Real ddensity_dp, Real relperm, Real drelperm_dp);

  /// second derivative of mobility wrt porepressure
  Real d2mobility_dp2(Real density, Real ddensity_dp, Real d2density_dp2, Real relperm, Real drelperm_dp, Real d2relperm_dp2);
};

#endif //RICHARDSFLUX
