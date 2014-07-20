
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

  /// Value of the yield function
  MaterialProperty<Real> & _yf;



  /**
   * Implements the return map
   * @param sig_old  The stress at the previous "time" step
   * @param plastic_strain_old  The value of plastic strain at the previous "time" step
   * @param delta_d  The total strain increment for this "time" step
   * @param E_ijkl   The elasticity tensor.  If no plasiticity then sig_new = sig_old + E_ijkl*delta_d
   * @param sig      The stress after returning to the yield surface   (this is an output variable)
   * @param plastic_strain   The value of plastic strain after returning to the yield surface (this is an output variable)
   * @param yield_fcn   The value of the yield function after returning to the yield surface (this is an output variable)
   * Note that this algorithm doesn't do any rotations.  In order to find the
   * final stress and plastic_strain, sig and plastic_strain must be rotated using _rotation_increment.
   */
  virtual void returnMap(const RankTwoTensor &sig_old, const RankTwoTensor &plastic_strain_old, const RankTwoTensor &delta_d, const RankFourTensor &E_ijkl, RankTwoTensor &sig, RankTwoTensor &plastic_strain, Real &yield_function);

  /**
   * Calculates the yield function
   * @param stress the stress at which to calculate the yield function
   * @param yield_stress the current value of the yield stress
   * @return equivalentstress - yield_stress
   */
  virtual Real yieldFunction(const RankTwoTensor &stress);


  /**
   * Derivative of yieldFunction with respect to the stress
   */
  virtual RankTwoTensor dyieldFunction_dstress(const RankTwoTensor &stress, const Real tan_fric_or_dil);


  /**
   * Flow potential, which in this case is just dyieldFunction_dstress
   * evaluated with the dilation angle rather than the friction angle
   */
  virtual RankTwoTensor flowPotential(const RankTwoTensor &stress);

  /**
   * Evaluates the derivative d(flowPotential_ij)/d(stress_kl), where
   * @param sig stress
   * @return the answer
   */
  virtual RankFourTensor dflowPotential_dstress(const RankTwoTensor &sig);

};

#endif //FINITESTRAINWEAKPLANESHEAR
