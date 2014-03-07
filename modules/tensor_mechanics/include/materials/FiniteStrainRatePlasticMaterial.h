// Original class author: A.M. Jokisaari,  O. Heinonen

#ifndef FINITESTRAINRATEPLASTICMATERIAL_H
#define FINITESTRAINRATEPLASTICMATERIAL_H

#include "FiniteStrainPlasticMaterial.h"

class FiniteStrainRatePlasticMaterial;

template<>
InputParameters validParams<FiniteStrainRatePlasticMaterial>();

class FiniteStrainRatePlasticMaterial : public FiniteStrainPlasticMaterial
{
public:
  FiniteStrainRatePlasticMaterial(const std:: string & name, InputParameters parameters);

protected:
  virtual void computeQpStress();
  virtual void initQpStatefulProperties();

  virtual void solveStressResid(RankTwoTensor,RankTwoTensor,RankFourTensor,RankTwoTensor*,RankTwoTensor*);
  void getJac(RankTwoTensor,RankFourTensor,Real,Real,RankFourTensor*);
  void getFlowTensor(RankTwoTensor,Real,RankTwoTensor*);

  Real _ref_pe_rate;
  Real _exponent;

  Real macaulayBracket(Real);

private:

};

#endif //FINITESTRAINRATEPLASTICMATERIAL_H
