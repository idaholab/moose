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
 * This class is an implementation of a generalized Kelvin-Voigt model
 * with constant mechanical properties. It consists of an arbitrary number
 * of Kelvin-Voigt units associated in series with a single spring, and an
 * optional dashpot (this could represent a Burgers model, for example).
 */
class GeneralizedKelvinVoigtModel : public GeneralizedKelvinVoigtBase
{
public:
  GeneralizedKelvinVoigtModel(const InputParameters & parameters);

protected:
  virtual void computeQpViscoelasticProperties();
  virtual void computeQpViscoelasticPropertiesInv();

  /**
   * The elasticity tensor associated with the first spring. This is
   * the true elasticity tensor of the material
   */
  RankFourTensor _C0;
  /// The elasticity tensor of each subsequent spring
  std::vector<RankFourTensor> _Ci;
  /// The viscosity of each dashpot
  std::vector<Real> _eta_i;

  /// The inverse of the elasticity tensor of the first spring
  RankFourTensor _S0;
  /// The inverse of each subsequent spring elasticity tensor
  std::vector<RankFourTensor> _Si;
};

#endif // GENERALIZEDKELVINVOIGTMODEL_H
