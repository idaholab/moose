/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GENERALIZEDKELVINVOIGTBASE_H
#define GENERALIZEDKELVINVOIGTBASE_H

#include "LinearViscoelasticityBase.h"

class GeneralizedKelvinVoigtBase;

template <>
InputParameters validParams<GeneralizedKelvinVoigtBase>();

/**
 * This class represents an assembly of springs and dashpots following
 * a generalized Kelvin-Voigt model (an arbitrary number of Kelvin-Voigt
 * units assembled in series with a single spring at the top).
 *
 * This class does not attribute the mechanical properties to each spring
 * and dashpot. It must be inherited to do so (see GeneralizedKelvinVoigtModel
 * for an example).
 *
 * This class derives from LinearViscoelasticityBase, and thus contains both
 * the apparent mechanical properties of the material, and the internal variables
 * associated to each of the dashpots in the model. It provides the methods
 * required to perform the time stepping scheme associated with viscoelastic models.
 *
 * See LinearViscoelasticityBase for more information
 */
class GeneralizedKelvinVoigtBase : public LinearViscoelasticityBase
{
public:
  GeneralizedKelvinVoigtBase(const InputParameters & parameters);

protected:
  virtual void computeQpApparentElasticityTensors() final;
  virtual void computeQpApparentCreepStrain() final;
  virtual void updateQpViscousStrains() final;
};

#endif // GENERALIZEDKELVINVOIGTBASE_H
