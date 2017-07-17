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

class GeneralizedKelvinVoigtModel : public GeneralizedKelvinVoigtBase
{
public:
  GeneralizedKelvinVoigtModel(const InputParameters & parameters);

protected:
  virtual void computeQpViscoelasticProperties();
  virtual void computeQpViscoelasticPropertiesInv();

  RankFourTensor _C0;
  std::vector<RankFourTensor> _Ci;
  std::vector<Real> _eta_i;

  RankFourTensor _S0;
  std::vector<RankFourTensor> _Si;
};

#endif // GENERALIZEDKELVINVOIGTMODEL_H
