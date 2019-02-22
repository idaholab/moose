#ifndef LINEARTEST7EQNFLUIDPROPERTIES_H
#define LINEARTEST7EQNFLUIDPROPERTIES_H

#include "TwoPhaseFluidProperties.h"

class LinearTest7EqnFluidProperties;

template <>
InputParameters validParams<LinearTest7EqnFluidProperties>();

/**
 * 7-equation fluid properties class used for testing derivatives
 *
 * This class is only for supplying simple, linear functions for saturation
 * temperature and interfacial density. It does not actually create the
 * single-phase fluid properties for the 2 phases.
 */
class LinearTest7EqnFluidProperties : public TwoPhaseFluidProperties
{
public:
  LinearTest7EqnFluidProperties(const InputParameters & parameters);

  virtual Real p_critical() const override;

  virtual Real p_sat(Real T) const override;

  virtual Real T_sat(Real p) const override;
  virtual Real dT_sat_dp(Real p) const override;

protected:
  /// Derivative of saturation temperature w.r.t. pressure
  const Real _dT_sat_dp;
  /// Derivative of interfacial density w.r.t. interfacial pressure
  const Real _drho_int_dp_int;
  /// Derivative of interfacial density w.r.t. interfacial temperature
  const Real _drho_int_dT_int;
};

#endif /* LINEARTEST7EQNFLUIDPROPERTIES_H */
