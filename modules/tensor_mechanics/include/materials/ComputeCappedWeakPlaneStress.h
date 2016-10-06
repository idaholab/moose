/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTECAPPEDWEAKPLANESTRESS_H
#define COMPUTECAPPEDWEAKPLANESTRESS_H

#include "ComputeStressBase.h"
#include "TensorMechanicsHardeningModel.h"

class ComputeCappedWeakPlaneStress;

template<>
InputParameters validParams<ComputeCappedWeakPlaneStress>();

/**
 * ComputeCappedWeakPlaneStress performs the return-map
 * algorithm and associated stress updates for plastic
 * models that describe capped weak-plane plasticity
 */
class ComputeCappedWeakPlaneStress :
  public ComputeStressBase
{
public:
  ComputeCappedWeakPlaneStress(const InputParameters & parameters);

protected:
  virtual void computeQpStress() override;
  virtual void initQpStatefulProperties() override;

  /// Hardening model for cohesion
  const TensorMechanicsHardeningModel & _cohesion;

  /// Hardening model for tan(phi)
  const TensorMechanicsHardeningModel & _tan_phi;

  /// Hardening model for tan(psi)
  const TensorMechanicsHardeningModel & _tan_psi;

  /// Hardening model for tensile strength
  const TensorMechanicsHardeningModel & _tstrength;

  /// Hardening model for compressive strength
  const TensorMechanicsHardeningModel & _cstrength;

  /// The cone vertex is smoothed by this amount
  const Real _small_smoother2;

  /// Maximum number of Newton-Raphson iterations allowed in the return-map process
  const unsigned _max_nr_its;

  /// Whether to perform finite-strain rotations
  const bool _perform_finite_strain_rotations;

  /// Smoothing tolerance: edges of the yield surface get smoothed by this amount
  const Real _smoothing_tol;

  /// The yield-function tolerance
  const Real _f_tol;

  /// Square of the yield-function tolerance
  const Real _f_tol2;

  /// The type of tangent operator to return.  tangent operator = d(stress_rate)/d(strain_rate).
  enum TangentOperatorEnum {
    elastic, nonlinear
  } _tangent_operator_type;

  /// plastic strain
  MaterialProperty<RankTwoTensor> & _plastic_strain;

  /// Old value of plastic strain
  MaterialProperty<RankTwoTensor> & _plastic_strain_old;

  /// internal parameters.  intnl[0] = shear.  intnl[1] = tensile.
  MaterialProperty<std::vector<Real> > & _intnl;

  /// old values of internal parameters
  MaterialProperty<std::vector<Real> > & _intnl_old;

  /// yield functions
  MaterialProperty<std::vector<Real> > & _yf;

  /// Number of Newton-Raphson iterations used in the return-map
  MaterialProperty<Real> & _iter;

  /// Whether a line-search was needed in the latest Newton-Raphson process (1 if true, 0 otherwise)
  MaterialProperty<Real> & _linesearch_needed;

  /// strain increment (coming from ComputeIncrementalSmallStrain, for example)
  const MaterialProperty<RankTwoTensor> & _strain_increment;

  /// Rotation increment (coming from ComputeIncrementalSmallStrain, for example)
  const MaterialProperty<RankTwoTensor> & _rotation_increment;

  /// Old value of stress
  MaterialProperty<RankTwoTensor> & _stress_old;

  /// Old value of elastic strain
  MaterialProperty<RankTwoTensor> & _elastic_strain_old;

  /** Computes the values of the yield functions, given stress and intnl
   * @param p stress_zz
   * @param q sqrt(stress_zx^2 + stress_zy^2)
   * @param intnl The internal parameters (intnl[0] is shear, intnl[1] is tensile)
   * @param[out] yf The yield function values (yf[0] = shear, yf[1] = tensile, yf[2] = compressive)
   */
  virtual void yieldFunctions(Real p, Real q, const std::vector<Real> & intnl, std::vector<Real> & yf);

  /** Computes d(yieldFunctions)/d(p, q)
   * @param p stress_zz
   * @param q sqrt(stress_zx^2 + stress_zy^2)
   * @param intnl The internal parameters (intnl[0] is shear, intnl[1] is tensile)
   * @param[out] dyf The yield function derivatives (dyf[0][0] = d(shear)/dp, dyf[0][1] = d(shear)/dq, etc)
   */
  virtual void dyieldFunctions(Real p, Real q, const std::vector<Real> & intnl, std::vector<std::vector<Real> > & dyf);

  /** Computes d(yieldFunctions)/d(intnl)
   * @param p stress_zz
   * @param q sqrt(stress_zx^2 + stress_zy^2)
   * @param intnl The internal parameters (intnl[0] is shear, intnl[1] is tensile)
   * @param[out] dyf The yield function derivatives (dyf[0][0] = d(shear)/dintnl[0], dyf[0][1] = d(shear)/dintnl[1], etc)
   */
  virtual void dyieldFunctions_di(Real p, Real q, const std::vector<Real> & intnl, std::vector<std::vector<Real> > & dyf);

  /** Computes the values of the flow potential, given stress and intnl
   * @param p stress_zz
   * @param q sqrt(stress_zx^2 + stress_zy^2)
   * @param intnl The internal parameters (intnl[0] is shear, intnl[1] is tensile)
   * @param[out] g The flow potential values (g[0] = shear, g[1] = tensile, g[2] = compressive)
   */
  virtual void flowPotential(Real p, Real q, const std::vector<Real> & intnl, std::vector<Real> & g);

  /** Computes d(flowPotential)/d(p, q)
   * @param p stress_zz
   * @param q sqrt(stress_zx^2 + stress_zy^2)
   * @param intnl The internal parameters (intnl[0] is shear, intnl[1] is tensile)
   * @param[out] dg The flow potential derivatives (dg[0][0] = d(shear)/dp, dg[0][1] = d(shear)/dq, etc)
   */
  virtual void dflowPotential(Real p, Real q, const std::vector<Real> & intnl, std::vector<std::vector<Real> > & dg);

  /** Computes d2(flowPotential)/d(p, q)/d(p, q)
   * @param p stress_zz
   * @param q sqrt(stress_zx^2 + stress_zy^2)
   * @param intnl The internal parameters (intnl[0] is shear, intnl[1] is tensile)
   * @param[out] d2g The flow potential derivatives (d2g[0][0][0] = d^2(shear)/dp/dp,
   *                 d2g[0][0][1] = d^2(shear)/dp/dq,
   *                 d2g[0][1][0] = d^2(shear)/dq/dp,
   *                 d2g[0][1][1] = d^2(shear)/dq/dq, etc)
   */
  virtual void d2flowPotential(Real p, Real q, const std::vector<Real> & intnl, std::vector<std::vector<std::vector<Real> > > & d2g);

  /** Computes d2(flowPotential)/d(p, q)/dintnl
   * @param p stress_zz
   * @param q sqrt(stress_zx^2 + stress_zy^2)
   * @param intnl The internal parameters (intnl[0] is shear, intnl[1] is tensile)
   * @param[out] d2g The flow potential derivatives (d2g[0][0][0] = d^2(shear)/dp/dintnl[0],
   *                 d2g[0][0][1] = d^2(shear)/dp/dintnl[1],
   *                 d2g[0][1][0] = d^2(shear)/dq/dintnl[0],
   *                 d2g[0][1][1] = d^2(shear)/dq/dintnl[1], etc)
   */
  virtual void d2flowPotential_di(Real p, Real q, const std::vector<Real> & intnl, std::vector<std::vector<std::vector<Real> > > & d2g);

  /** Computes the smoothed yield function
   * @param p stress_zz
   * @param q sqrt(stress_zx^2 + stress_zy^2)
   * @param intnl The internal parameters (intnl[0] is shear, intnl[1] is tensile)
   * @return The smoothed yield function
   */
  virtual Real yieldF(Real p, Real q, const std::vector<Real> & intnl);

  /** Performs smoothing of yield functions.  Note that f gets modified
   * by this function.  Upon return, f.back() will be the smoothed yield-function value
   * @param f The input yield-function values
   */
  virtual void smooth(std::vector<Real> & f) const;

  /** Smooths yield functions
   */
  Real ismoother(Real f_diff) const;

  /** Derivative of ismoother
   */
  Real smoother(Real f_diff) const;

  /** Derivative of smoother
   */
  Real dsmoother(Real f_diff) const;

  /** Performs the return-map algorithm
   */
  virtual void returnMap();

  struct f_and_derivs
  {
    Real f;
    std::vector<Real> df;
    std::vector<Real> df_di;
    std::vector<Real> dg;
    std::vector<std::vector<Real> > d2g;
    std::vector<std::vector<Real> > d2g_di;

    f_and_derivs(Real f_in,
                 const std::vector<Real> & df_in,
                 const std::vector<Real> & df_di_in,
                 const std::vector<Real> & dg_in,
                 const std::vector<std::vector<Real> > & d2g_in,
                 const std::vector<std::vector<Real> > & d2g_di_in):
      f(f_in),
      df(2),
      df_di(2),
      dg(2),
      d2g(2),
      d2g_di(2)
    {
      for (unsigned i = 0; i < 2; ++i)
      {
        df[i] = df_in[i];
        df_di[i] = df_di_in[i];
        dg[i] = dg_in[i];
        d2g[i].resize(2);
        d2g_di[i].resize(2);
        for (unsigned j = 0; j < 2; ++j)
        {
          d2g[i][j] = d2g_in[i][j];
          d2g_di[i][j] = d2g_di_in[i][j];
        }
      }
    }

    bool operator < (const f_and_derivs & fd) const
    {
      return f < fd.f;
    }
  };

  f_and_derivs smoothAllQuantities(Real p, Real q, const std::vector<Real> & intnl);


};

#endif //COMPUTECAPPEDWEAKPLANESTRESS_H
