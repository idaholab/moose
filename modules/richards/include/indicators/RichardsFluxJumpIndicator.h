/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSFLUXJUMPINDICATOR_H
#define RICHARDSFLUXJUMPINDICATOR_H

#include "JumpIndicator.h"
#include "RichardsVarNames.h"

class RichardsFluxJumpIndicator;

template<>
InputParameters validParams<RichardsFluxJumpIndicator>();

/**
 * Jump of the Richards mass-flux
 * = density * relative_permeability * permeability * (grad(porepressure) - density * gravity)/viscosity
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
   * holds info regarding the names of the richards variables
   * and methods for extracting values of these variables
   */
  const RichardsVarNames & _richards_name_UO;

  /**
   * the index of this variable in the list of richards variables
   * held by _richards_name_UO.  Eg
   * if richards_vars = 'pwater pgas poil' in the _richards_name_UO
   * and this kernel has variable = pgas, then _pvar = 1
   * This is used to index correctly into seff_UO, sat_UO, density_UO, etc.
   */
  unsigned int _pvar;

  /// fluid flux (vector of fluxes if multicomponent)
  MaterialProperty<std::vector<RealVectorValue> > &_flux;

  /// fluid flux at neighbours
  MaterialProperty<std::vector<RealVectorValue> > &_flux_n;

};

#endif /* RICHARDSFLUXJUMPINDICATOR_H */
