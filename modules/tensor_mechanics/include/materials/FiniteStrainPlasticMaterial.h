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



  RankTwoTensor solveStressResid(RankTwoTensor,RankTwoTensor,RankFourTensor,RankTwoTensor*);
  unsigned int isPlastic(RankTwoTensor,Real);
  RankTwoTensor getFlowTensor(RankTwoTensor);
  Real getSigEqv(RankTwoTensor);
  RankTwoTensor getSigDev(RankTwoTensor);
  RankFourTensor getJac(RankTwoTensor,RankFourTensor,Real);
  Real deltaFunc(int,int);
  Real getYieldStress(Real);
  
  
private:

};

#endif //FINITESTRAINPLASTICMATERIAL_H
