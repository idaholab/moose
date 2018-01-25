//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEISOTROPICLINEARELASTICPFFRACTURESTRESS_H
#define COMPUTEISOTROPICLINEARELASTICPFFRACTURESTRESS_H

#include "ComputeStressBase.h"

class ComputeIsotropicLinearElasticPFFractureStress;

template <>
InputParameters validParams<ComputeIsotropicLinearElasticPFFractureStress>();

/**
 * Phase-field fracture
 * This class computes the stress and energy contribution for the
 * small strain Isotropic Elastic formulation of phase field fracture
 */
class ComputeIsotropicLinearElasticPFFractureStress : public ComputeStressBase
{
public:
  ComputeIsotropicLinearElasticPFFractureStress(const InputParameters & parameters);

protected:
  /// Function required to initialize statefull material properties
  virtual void initQpStatefulProperties();

  /// Primary funciton of this material, computes stress and elastic energy
  virtual void computeQpStress();

  /// Coupled order parameter defining the crack
  const VariableValue & _c;

  /// Small number to avoid non-positive definiteness at or near complete damage
  const Real _kdamage;

  /// Material property defining crack width, declared elsewhere
  const MaterialProperty<Real> & _l;

  /// Material property defining gc parameter, declared elsewhere
  const MaterialProperty<Real> & _gc;

  /// Elastic energy and derivatives, declared in this material
  MaterialProperty<Real> & _F;
  MaterialProperty<Real> & _dFdc;
  MaterialProperty<Real> & _d2Fdc2;
  MaterialProperty<RankTwoTensor> & _d2Fdcdstrain;
  MaterialProperty<RankTwoTensor> & _dstress_dc;

  /// History variable that prevents crack healing, declared in this material
  MaterialProperty<Real> & _hist;

  /// Old value of history variable
  const MaterialProperty<Real> & _hist_old;
};

#endif // COMPUTEISOTROPICLINEARELASTICPFFRACTURESTRESS_H
