/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSMASSCHANGE
#define RICHARDSMASSCHANGE

#include "TimeDerivative.h"
#include "RichardsVarNames.h"

// Forward Declarations
class RichardsMassChange;

template<>
InputParameters validParams<RichardsMassChange>();

/**
 * Kernel = (mass - mass_old)/dt
 * where mass = porosity*density*saturation
 * This is used for the time derivative in Richards simulations
 * Note that it is not lumped, so usually you want to use RichardsLumpedMassChange instead
 */
class RichardsMassChange : public TimeDerivative
{
public:

  RichardsMassChange(const std::string & name,
                        InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// holds info on the Richards variables
  const RichardsVarNames & _richards_name_UO;

  /**
   * the Richards variable number
   * eg, if richards name = 'pwater pgas poil', and this
   * kernel is for pgas, then _pvar = 1
   */
  unsigned int _pvar;

  /// whether to use SUPG for this kernel (not recommended)
  bool _use_supg;

  /// material porosity
  MaterialProperty<Real> &_porosity;

  /// old material porosity
  MaterialProperty<Real> &_porosity_old;

  /// old saturation(s)
  MaterialProperty<std::vector<Real> > &_sat_old;

  /// saturation(s)
  MaterialProperty<std::vector<Real> > &_sat;

  /// derivatives of saturation(s) wrt effective saturation(s)
  MaterialProperty<std::vector<std::vector<Real> > > &_dsat;

  /// second derivatives of saturation(s) wrt effective saturation(s)
  MaterialProperty<std::vector<std::vector<std::vector<Real> > > > &_d2sat;

  /// old values of density
  MaterialProperty<std::vector<Real> > &_density_old;

  /// fluid density (vector of densities in the case of multiphase flow)
  MaterialProperty<std::vector<Real> > &_density;

  /// derivative of fluid density wrt pressure (deriv of densities wrt their pressures in the case of multiphase flow)
  MaterialProperty<std::vector<Real> > &_ddensity;

  /// second derivative of fluid density wrt pressure (second deriv of densities wrt their pressures in the case of multiphase flow)
  MaterialProperty<std::vector<Real> > &_d2density;

  /// tau_SUPG
  MaterialProperty<std::vector<RealVectorValue> >&_tauvel_SUPG;

  /// derivative of tau_SUPG wrt gradp
  MaterialProperty<std::vector<RealTensorValue> >&_dtauvel_SUPG_dgradp;

  /// deriv of tau_SUPG wrt p
  MaterialProperty<std::vector<RealVectorValue> >&_dtauvel_SUPG_dp;

};

#endif //RICHARDSMASSCHANGE
