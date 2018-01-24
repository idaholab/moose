//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ACGRGRELASTICDRIVINGFORCE_H
#define ACGRGRELASTICDRIVINGFORCE_H

#include "ACBulk.h"

// Forward Declarations
class ACGrGrElasticDrivingForce;
class RankTwoTensor;
class RankFourTensor;

template <>
InputParameters validParams<ACGrGrElasticDrivingForce>();

/**
 * Calculates the porton of the Allen-Cahn equation that results from the deformation energy.
 * Must access the elastic_strain stored as a material property
 * Requires the name of the elastic tensor derivative as an input.
 */
class ACGrGrElasticDrivingForce : public ACBulk<Real>
{
public:
  ACGrGrElasticDrivingForce(const InputParameters & parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);

private:
  const MaterialProperty<RankFourTensor> & _D_elastic_tensor;
  const MaterialProperty<RankTwoTensor> & _elastic_strain;
};

#endif // ACGRGRELASTICDRIVINGFORCE_H
