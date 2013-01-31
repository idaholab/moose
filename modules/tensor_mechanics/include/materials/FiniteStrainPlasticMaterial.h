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

  Real _yield_stress;
  MaterialProperty<RankTwoTensor> & _plastic_strain;
  MaterialProperty<RankTwoTensor> & _plastic_strain_old;


  RankTwoTensor solveStressResid(RankTwoTensor,RankTwoTensor,RankFourTensor);

  unsigned int isPlastic(RankTwoTensor);
  
  RankTwoTensor getFlowTensor(RankTwoTensor);
  Real getSigEqv(RankTwoTensor);
  RankTwoTensor getSigDev(RankTwoTensor);
  RankFourTensor getJac(RankTwoTensor,RankFourTensor,Real);
  Real deltaFunc(int,int);
  
  
private:

};

#endif //FINITESTRAINPLASTICMATERIAL_H
