#ifndef ONEDSTAGNATIONPANDTBASE_H
#define ONEDSTAGNATIONPANDTBASE_H

#include "Function.h"

class SinglePhaseFluidProperties;

/**
 * Provides common functions for stagnation pressure and temperature BC.
 */
class OneDStagnationPandTBase
{
public:
  OneDStagnationPandTBase(const SinglePhaseFluidProperties & fp);

  /**
   * Computes static density and static pressure from stagnation pressure, stagnation temperature,
   * and velocity
   *
   * @param[in] p0    stagnation pressure
   * @param[in] T0    stagnation temperature
   * @param[in] vel   velocity
   * @param[out] rho  static density
   * @param[out] p    static pressure
   */
  void rho_p_from_p0_T0_vel(Real p0, Real T0, Real vel, Real & rho, Real & p);

  /**
   * Computes derivative of static pressure w.r.t. velocity from stagnation pressure, stagnation
   * temperature, and velocity
   *
   * @param[in] p0    stagnation pressure
   * @param[in] T0    stagnation temperature
   * @param[in] vel   velocity
   *
   * @return derivative of static pressure w.r.t. velocity
   */
  Real dpdu_from_p0_T0_vel(Real p0, Real T0, Real vel);

  /**
   * Computes derivative of static density w.r.t. velocity from stagnation pressure, stagnation
   * temperature, and velocity
   *
   * @param[in] p0    stagnation pressure
   * @param[in] T0    stagnation temperature
   * @param[in] vel   velocity
   *
   * @return derivative of static density w.r.t. velocity
   */
  Real drhodu_from_p0_T0_vel(Real p0, Real T0, Real vel);

  /**
   * Computes derivative of static total energy w.r.t. velocity from stagnation pressure, stagnation
   * temperature, and velocity
   *
   * @param[in] p0    stagnation pressure
   * @param[in] T0    stagnation temperature
   * @param[in] vel   velocity
   *
   * @return derivative of static total energy w.r.t. velocity
   */
  Real dEdu_from_p0_T0_vel(Real p0, Real T0, Real vel);

protected:
  const SinglePhaseFluidProperties & _fp;
};

#endif // ONEDSTAGNATIONPANDTBASE
