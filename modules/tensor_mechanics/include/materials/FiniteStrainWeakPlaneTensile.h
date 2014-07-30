
#ifndef FINITESTRAINWEAKPLANETENSILE
#define FINITESTRAINWEAKPLANETENSILE

#include "FiniteStrainPlasticBase.h"

class FiniteStrainWeakPlaneTensile;

template<>
InputParameters validParams<FiniteStrainWeakPlaneTensile>();

/**
 * FiniteStrainWeakPlaneTensile implements rate-independent associative weak-plane tensile failure
 * with no hardening in the finite-strain framework.
 */
class FiniteStrainWeakPlaneTensile : public FiniteStrainPlasticBase
{
public:
  FiniteStrainWeakPlaneTensile(const std::string & name, InputParameters parameters);

protected:
  virtual void initQpStatefulProperties();

  /// tension cutoff
  Real _tension_cutoff;

  /// Unit normal inputted by user
  RealVectorValue _input_n;

  /// Whether the normal vector rotates with large deformations
  bool _normal_rotates;

  /// Unit normal vector to the weak plane
  MaterialProperty<RealVectorValue> & _n;

  /// Old value of unit normal vector to the weak plane
  MaterialProperty<RealVectorValue> & _n_old;

  /// Value of the yield function
  MaterialProperty<Real> & _yf;

  /**
   * The yield function(s)
   * @param stress the stress at which to calculate the yield function
   * @param intnl vector of internal parameters
   * @param f (output) the yield function (or functions in the case of multisurface plasticity)
   */
  virtual void yieldFunction(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<Real> & f);

  /**
   * The derivative of yield function(s) with respect to stress
   * @param stress the stress at which to calculate the yield function
   * @param intnl vector of internal parameters
   * @param df_dstress (output) the derivative (or derivatives in the case of multisurface plasticity).  df_dstress[alpha](i, j) = dyieldFunction[alpha]/dstress(i, j)
   */
  virtual void dyieldFunction_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<RankTwoTensor> & df_dstress);

  /**
   * The flow potential(s) - one for each yield function
   * @param stress the stress at which to calculate the flow potential
   * @param intnl vector of internal parameters
   * @param r (output) the flow potential (flow potentials in the multi-surface case)
   */
  virtual void flowPotential(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<RankTwoTensor> & r);

  /**
   * The derivative of the flow potential(s) with respect to stress
   * @param stress the stress at which to calculate the flow potential
   * @param intnl vector of internal parameters
   * @param dr_dstress (output) the derivative.  dr_dstress[alpha](i, j, k, l) = dr[alpha](i, j)/dstress(k, l)
   */
  virtual void dflowPotential_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, std::vector<RankFourTensor> & dr_dstress);

  /// Function called just before doing returnMap
  virtual void preReturnMap();

  /// Function called just after doing returnMap
  virtual void postReturnMap();
};

#endif //FINITESTRAINWEAKPLANETENSILE
