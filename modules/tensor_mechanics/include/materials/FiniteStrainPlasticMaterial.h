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
  FiniteStrainPlasticMaterial(const std:: string & name, InputParameters parameters);

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

  virtual void solveStressResid(RankTwoTensor,RankTwoTensor,RankFourTensor,RankTwoTensor*,RankTwoTensor*);
  void getJac(RankTwoTensor,RankFourTensor,Real,RankFourTensor*);
  void getFlowTensor(RankTwoTensor,RankTwoTensor*);


  unsigned int isPlastic(RankTwoTensor,Real);

  Real getSigEqv(RankTwoTensor);
  RankTwoTensor getSigDev(RankTwoTensor);

  Real deltaFunc(int,int);
  Real getYieldStress(Real);
  Real getdYieldStressdPlasticStrain(Real);


private:

};

#endif //FINITESTRAINPLASTICMATERIAL_H
