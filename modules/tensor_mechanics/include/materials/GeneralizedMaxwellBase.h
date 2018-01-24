/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GENERALIZEDMAXWELLBASE_H
#define GENERALIZEDMAXWELLBASE_H

#include "LinearViscoelasticityBase.h"

class GeneralizedMaxwellBase;

template <>
InputParameters validParams<GeneralizedMaxwellBase>();

/**
 * This class represents an assembly of springs and dashpots following
 * a generalized Maxwell model (an arbitrary number of Maxwell
 * units assembled in parallel with a single spring).
 *
 * This class does not attribute the mechanical properties to each spring
 * and dashpot. It must be inherited to do so (see GeneralizedMaxwellModel
 * for an example).
 *
 * This class derives from LinearViscoelasticityBase, and thus contains both
 * the apparent mechanical properties of the material, and the internal variables
 * associated to each of the dashpots in the model. It provides the methods
 * required to perform the time stepping scheme associated with viscoelastic models.
 *
 * See LinearViscoelasticityBase for more information
 */
class GeneralizedMaxwellBase : public LinearViscoelasticityBase
{
public:
  GeneralizedMaxwellBase(const InputParameters & parameters);

protected:
  virtual void computeQpApparentElasticityTensors() final;
  virtual void computeQpApparentCreepStrain() final;
  virtual void updateQpViscousStrains() final;
};

#endif // GENERALIZEDMAXWELLBASE_H
