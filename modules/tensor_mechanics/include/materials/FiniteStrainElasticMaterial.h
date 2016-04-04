/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
// Original class author: A.M. Jokisaari,  O. Heinonen

#ifndef FINITESTRAINELASTICMATERIAL_H
#define FINITESTRAINELASTICMATERIAL_H

#include "FiniteStrainMaterial.h"
#include "Function.h"

/**
 * FiniteStrainElasticMaterial handles a fully anisotropic, single-crystal material's elastic
 * constants.  It takes all 21 independent stiffness tensor inputs, or only 9, depending on the
 * boolean input value given.  This can be extended or simplified to specify HCP, monoclinic,
 * cubic, etc as needed.
 */
class FiniteStrainElasticMaterial : public FiniteStrainMaterial
{
public:
  FiniteStrainElasticMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpStress();
  virtual void computeQpElasticityTensor();
  ElasticityTensorR4 _Cijkl;
  Function * const _prefactor_function;
};

#endif //FINITESTRAINELASTICMATERIAL_H
