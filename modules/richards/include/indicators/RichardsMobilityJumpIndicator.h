/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSMOBILITYJUMPINDICATOR_H
#define RICHARDSMOBILITYJUMPINDICATOR_H

#include "JumpIndicator.h"
#include "RichardsVarNames.h"

class RichardsMobilityJumpIndicator;

template<>
InputParameters validParams<RichardsMobilityJumpIndicator>();

/**
 * Jump of the Richards mobility
 * mobility = density * relative_permeability/visocity
 * jump = (mobility - mobility_neighbor)/(mobility + mobility_neighbor + a)^b
 */
class RichardsMobilityJumpIndicator :
  public JumpIndicator
{
public:
  RichardsMobilityJumpIndicator(const std::string & name, InputParameters parameters);
  virtual ~RichardsMobilityJumpIndicator(){};

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

  /// jump = (mobility - mobility_neighbor)/(mobility + mobility_neighbor + a)^b
  Real _a;

  /// jump = (mobility - mobility_neighbor)/(mobility + mobility_neighbor + a)^b
  Real _b;

  /// fluid relative permeability (vector of relperms if multicomponent)
  MaterialProperty<std::vector<Real> > &_rel_perm;

  /// fluid density (vector of densities if multicomponent)
  MaterialProperty<std::vector<Real> > &_density;

  /// fluid viscosity (vector of densities if multicomponent)
  MaterialProperty<std::vector<Real> > &_viscosity;

  /// neighbor fluid relative permeability (vector of relperms if multicomponent)
  MaterialProperty<std::vector<Real> > &_rel_perm_n;

  /// neighbor fluid density (vector of densities if multicomponent)
  MaterialProperty<std::vector<Real> > &_density_n;

  /// neighbor fluid viscosity (vector of densities if multicomponent)
  MaterialProperty<std::vector<Real> > &_viscosity_n;

};

#endif /* RICHARDSMOBILITYJUMPINDICATOR_H */
