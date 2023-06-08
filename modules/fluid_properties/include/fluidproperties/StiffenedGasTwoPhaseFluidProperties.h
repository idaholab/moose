#pragma once

#include "TwoPhaseFluidProperties.h"
#include "LinearInterpolation.h"
#include "NaNInterface.h"

class StiffenedGasFluidProperties;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

/**
 * Two-phase stiffened gas fluid properties
 */
class StiffenedGasTwoPhaseFluidProperties : public TwoPhaseFluidProperties, public NaNInterface
{
public:
  StiffenedGasTwoPhaseFluidProperties(const InputParameters & parameters);

  virtual Real p_critical() const override;
  virtual Real T_triple() const override;
  virtual Real L_fusion() const override;
  virtual Real T_sat(Real pressure) const override;
  virtual Real p_sat(Real temperature) const override;
  virtual Real dT_sat_dp(Real pressure) const override;
  virtual Real sigma_from_T(Real T) const override;
  virtual Real dsigma_dT_from_T(Real T) const override;

  virtual bool supportsPhaseChange() const override { return true; }

protected:
  /**
   * Computes saturation pressure value using Newton solve
   *
   * The process for determining the saturation pressure is given in the following reference:
   *
   * Ray A. Berry, Richard Saurel, Olivier LeMetayer
   * The discrete equation method (DEM) for fully compressible, two-phase flow
   *   in ducts of spatially varying cross-section
   * Nuclear Engineering and Design 240 (2010) p. 3797-3818
   *
   * The nonlinear equation to be solved is given by Equation (38) of this
   * reference; it is obtained by reasoning that at thermodynamic equilibrium,
   * the Gibbs free enthalpy of the phases must be equal at the interface.
   *
   * @param[in] T  temperature at which saturation pressure is to be computed
   */
  Real compute_p_sat(const Real & T) const;

  // liquid SGEOS parameters
  const Real _gamma_liquid;
  const Real _cv_liquid;
  const Real _cp_liquid;
  const Real _q_liquid;
  const Real _p_inf_liquid;
  const Real _q_prime_liquid;

  // vapor SGEOS parameters
  const Real _gamma_vapor;
  const Real _cv_vapor;
  const Real _cp_vapor;
  const Real _q_vapor;
  const Real _p_inf_vapor;
  const Real _q_prime_vapor;

  /// critical temperature
  const Real & _T_c;
  /// critical pressure
  const Real & _p_c;
  /// Triple-point temperature
  const Real & _T_triple;
  /// Latent heat of fusion
  const Real & _L_fusion;

  /// 'A' constant used in surface tension correlation
  const Real & _sigma_A;
  /// 'B' constant used in surface tension correlation
  const Real & _sigma_B;
  /// 'C' constant used in surface tension correlation
  const Real & _sigma_C;

  /// Minimum temperature value in saturation curve
  const Real & _T_sat_min;
  /// Maximum temperature value in saturation curve
  const Real & _T_sat_max;
  /// Initial guess for saturation pressure Newton solve
  const Real & _p_sat_guess;
  /// Number of samples to take in saturation curve
  const unsigned int & _n_sat_samples;
  /// Temperature increments on saturation curve
  const Real _dT_sat;

  // coefficients for saturation pressure Newton solve
  const Real _A;
  const Real _B;
  const Real _C;
  const Real _D;

  /// Newton solve tolerance
  const Real _newton_tol;
  /// Newton max number of iterations
  const unsigned int _newton_max_iter;

  // These two vectors store saturation line p(T) information by only calculating
  // once in constructor and then use interpolation to quickly calculate the value
  std::vector<Real> _T_vec;
  std::vector<Real> _p_sat_vec;

  LinearInterpolation _ipol_temp;
  LinearInterpolation _ipol_pressure;

public:
  static InputParameters validParams();
};

#pragma GCC diagnostic pop
