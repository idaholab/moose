//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearViscoelasticityBase.h"

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
  static InputParameters validParams();

  GeneralizedMaxwellBase(const InputParameters & parameters);

protected:
  virtual void computeQpApparentElasticityTensors() final;
  virtual void computeQpApparentCreepStrain() final;
  virtual void updateQpViscousStrains() final;
};
