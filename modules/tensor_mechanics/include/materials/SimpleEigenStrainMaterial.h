/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
// Original class author: A.M. Jokisaari,  O. Heinonen

#ifndef SIMPLEEIGENSTRAINMATERIAL_H
#define SIMPLEEIGENSTRAINMATERIAL_H

#include "EigenStrainBaseMaterial.h"

/**
 * SimpleEigenStrainMaterial handles a fully anisotropic, single-crystal material's elastic
 * constants.  It takes all 21 independent stiffness tensor inputs, or only 9, depending on the
 * boolean input value given.  This can be extended or simplified to specify HCP, monoclinic,
 * cubic, etc as needed.
 */
class SimpleEigenStrainMaterial : public EigenStrainBaseMaterial
{
public:
  SimpleEigenStrainMaterial(const InputParameters & parameters);

protected:
  virtual void computeEigenStrain();
  virtual void computeQpElasticityTensor();

private:
  Real _epsilon0;
  Real _c0;
};

#endif // SIMPLEEIGENSTRAINMATERIAL_H
