
#ifndef FINITESTRAINWEAKPLANESHEAR
#define FINITESTRAINWEAKPLANESHEAR

#include "FiniteStrainMaterial.h"

class FiniteStrainWeakPlaneShear;

template<>
InputParameters validParams<FiniteStrainWeakPlaneShear>();

/**
 * FiniteStrainWeakPlaneShear implements rate-independent non-associative weak-plane Mohr-Coulomb
 * with no hardening in the finite-strain framework.
 */
class FiniteStrainWeakPlaneShear : public FiniteStrainMaterial
{
public:
  FiniteStrainWeakPlaneShear(const std::string & name, InputParameters parameters);

protected:
  virtual void computeQpStress();
  virtual void initQpStatefulProperties();

  /// The cohesion
  Real _cohesion;

  /// tan(friction angle)
  Real _tan_phi;

  /// tan(dilation angle)
  Real _tan_psi;

  /// The cohesion_residual
  Real _cohesion_residual;

  /// tan(friction angle_residual)
  Real _tan_phi_residual;

  /// tan(dilation angle_residual)
  Real _tan_psi_residual;

  /// Logarithmic rate of change of cohesion to _cohesion_residual
  Real _cohesion_rate;

  /// Logarithmic rate of change of tan_phi to tan_phi_residual
  Real _tan_phi_rate;

  /// Logarithmic rate of change of tan_psi to tan_psi_residual
  Real _tan_psi_rate;

  /// Unit normal inputted by user
  RealVectorValue _input_n;

  /// Whether the normal vector rotates with large deformations
  bool _normal_rotates;

  /// Tolerance for yield function
  Real _f_tol;

  /// Tolerance for residual
  Real _r_tol;

  /**
   * The yield function needs to be smooth around shear-stress=0,
   * so it is modified to be
   * f = sqrt(s_xz^2 + s_yz^2 + (_small_smoother*_cohesion)^2) + s_zz*_tan_phi - _cohesion
   */
  Real _small_smoother;

  /// Maximum number of Newton-Raphson iterations allowed
  int _max_iter;

  /// plastic strain
  MaterialProperty<RankTwoTensor> & _plastic_strain;

  /// Old value of plastic strain
  MaterialProperty<RankTwoTensor> & _plastic_strain_old;

  /// Unit normal vector to the weak plane
  MaterialProperty<RealVectorValue> & _n;

  /// Old value of unit normal vector to the weak plane
  MaterialProperty<RealVectorValue> & _n_old;

  /// Accumulated plastic strain (the internal parameter), used in hardening relationship
  MaterialProperty<Real> & _wps_internal;

  /// Old value of accumulated plastic strain (the internal parameter), used in hardening relationship
  MaterialProperty<Real> & _wps_internal_old;

  /// Value of the yield function
  MaterialProperty<Real> & _yf;



  /**
   * Implements the return map
   * @param sig_old  The stress at the previous "time" step
   * @param plastic_strain_old  The value of plastic strain at the previous "time" step
   * @param internal_old The value of the internal parameter at the previous "time" step
   * @param delta_d  The total strain increment for this "time" step
   * @param E_ijkl   The elasticity tensor.  If no plasiticity then sig_new = sig_old + E_ijkl*delta_d
   * @param sig      The stress after returning to the yield surface   (this is an output variable)
   * @param plastic_strain   The value of plastic strain after returning to the yield surface (this is an output variable)
   * @param internal The value of the internal parameter after returning to the yield surface (this is an output variable)
   * @param yield_fcn   The value of the yield function after returning to the yield surface (this is an output variable)
   * Note that this algorithm doesn't do any rotations.  In order to find the
   * final stress and plastic_strain, sig and plastic_strain must be rotated using _rotation_increment.
   */
  virtual void returnMap(const RankTwoTensor &sig_old, const RankTwoTensor &plastic_strain_old, const Real &internal_old, const RankTwoTensor &delta_d, const RankFourTensor &E_ijkl, RankTwoTensor &sig, RankTwoTensor &plastic_strain, Real &internal, Real &yield_function);

  /**
   * Calculates the yield function
   * @param stress the stress at which to calculate the yield function
   * @param yield_stress the current value of the yield stress
   * @return equivalentstress - yield_stress
   */
  virtual Real yieldFunction(const RankTwoTensor &stress, const Real internal_param);


  /**
   * Derivative of yieldFunction with respect to the stress
   */
  virtual RankTwoTensor dyieldFunction_dstress(const RankTwoTensor &stress, const Real tan_fric_or_dil);


  /**
   * Derivative of yieldFunction with respect to internal parameter
   */
  virtual Real dyieldFunction_dinternal(const RankTwoTensor &stress, const Real internal_param);


  /**
   * Flow potential, which in this case is just dyieldFunction_dstress
   * evaluated with the dilation angle rather than the friction angle
   */
  virtual RankTwoTensor flowPotential(const RankTwoTensor &stress, const Real internal_param);

  /**
   * Evaluates the derivative d(flowPotential_ij)/d(stress_kl), where
   * @param sig stress
   * @return the answer
   */
  virtual RankFourTensor dflowPotential_dstress(const RankTwoTensor &sig, const Real /*internal_param*/);

  /**
   * d(flowPotential_ij)/d(internal_param)
   */
  virtual RankTwoTensor dflowPotential_dinternal(const RankTwoTensor &/*stress*/, const Real internal_param);


  /// cohesion as a function of internal parameter
  Real cohesion(const Real internal_param);

  /// d(cohesion)/d(internal_param);
  Real dcohesion(const Real internal_param);

  /// tan_phi as a function of internal parameter
  Real tan_phi(const Real internal_param);

  /// d(tan_phi)/d(internal_param);
  Real dtan_phi(const Real internal_param);

  /// tan_psi as a function of internal parameter
  Real tan_psi(const Real internal_param);

  /// d(tan_psi)/d(internal_param);
  Real dtan_psi(const Real internal_param);

};

#endif //FINITESTRAINWEAKPLANESHEAR
