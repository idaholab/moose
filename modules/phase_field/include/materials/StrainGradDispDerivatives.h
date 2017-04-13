/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef STRAINGRADDISPDERIVATIVES_H
#define STRAINGRADDISPDERIVATIVES_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"

class RankTwoTensor;
class StrainGradDispDerivatives;

template <>
InputParameters validParams<StrainGradDispDerivatives>();

class StrainGradDispDerivatives : public DerivativeMaterialInterface<Material>
{
public:
  StrainGradDispDerivatives(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  unsigned int _nvar;
  unsigned int _gdim;

  std::vector<MaterialProperty<RankTwoTensor> *> _dstrain;
};

#endif // STRAINGRADDISPDERIVATIVES_H
