/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GENERALIZEDKELVINVOIGTMODEL_H
#define GENERALIZEDKELVINVOIGTMODEL_H

#include "GeneralizedKelvinVoigtBase.h"

class GeneralizedKelvinVoigtModel;

template <>
InputParameters validParams<GeneralizedKelvinVoigtModel>();

/**
 * Represents a generalized Kelvin-Voigt model with constant and isotropic
 * mechanical properties.
 */
class GeneralizedKelvinVoigtModel : public GeneralizedKelvinVoigtBase
{
public:
  GeneralizedKelvinVoigtModel(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpViscoelasticProperties();

  RankFourTensor _C0;
  std::vector<RankFourTensor> _Ci;
  std::vector<Real> _eta_i;
};

#endif // GENERALIZEDKELVINVOIGTMODEL_H
