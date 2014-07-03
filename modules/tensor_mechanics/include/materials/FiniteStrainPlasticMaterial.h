// Original class author: A.M. Jokisaari,  O. Heinonen

#ifndef FINITESTRAINPLASTICMATERIAL_H
#define FINITESTRAINPLASTICMATERIAL_H

#include "FiniteStrainMaterial.h"

class FiniteStrainPlasticMaterial;

template<>
InputParameters validParams<FiniteStrainPlasticMaterial>();

class FiniteStrainPlasticMaterial : public FiniteStrainMaterial
{
public:
  FiniteStrainPlasticMaterial(const std::string & name, InputParameters parameters);

protected:
  virtual void computeQpStress();
  virtual void initQpStatefulProperties();

  std::vector<Real> _yield_stress_vector;
  MaterialProperty<RankTwoTensor> & _plastic_strain;
  MaterialProperty<RankTwoTensor> & _plastic_strain_old;
  MaterialProperty<Real> & _eqv_plastic_strain;
  MaterialProperty<Real> & _eqv_plastic_strain_old;
  Real _rtol;
  Real _ftol;
  Real _eptol;

  virtual void solveStressResid(const RankTwoTensor &, const RankTwoTensor &, const RankFourTensor &, RankTwoTensor &, RankTwoTensor &);
  void getJac(const RankTwoTensor &, const RankFourTensor &, Real, RankFourTensor &);
  void getFlowTensor(const RankTwoTensor &, RankTwoTensor &);

  unsigned int isPlastic(const RankTwoTensor &, Real);

  Real getSigEqv(const RankTwoTensor &);
  RankTwoTensor getSigDev(const RankTwoTensor &);

  Real deltaFunc(unsigned int, unsigned int);
  Real getYieldStress(Real);
  Real getdYieldStressdPlasticStrain(Real);
};

#endif //FINITESTRAINPLASTICMATERIAL_H
