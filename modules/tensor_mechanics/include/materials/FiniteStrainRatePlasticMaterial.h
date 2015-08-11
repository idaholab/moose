/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
  FiniteStrainRatePlasticMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpStress();
  virtual void initQpStatefulProperties();

  // Avoid warnings about hidden overloaded virtual function
  using FiniteStrainPlasticMaterial::returnMap;
  using FiniteStrainPlasticMaterial::getJac;

  virtual void returnMap(const RankTwoTensor &, const RankTwoTensor &, const RankFourTensor &, RankTwoTensor &, RankTwoTensor &);
  void getJac(const RankTwoTensor &, const RankFourTensor &, Real, Real, RankFourTensor &);
  void getFlowTensor(const RankTwoTensor &, Real, RankTwoTensor &);

  Real _ref_pe_rate;
  Real _exponent;

  Real macaulayBracket(Real);
};

#endif //FINITESTRAINRATEPLASTICMATERIAL_H
