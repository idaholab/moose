//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADMATERIAL_H
#define ADMATERIAL_H

#include "Material.h"
#include "MooseTypes.h"

#define usingMaterialMembers using ADMaterial<compute_stage>::_qp

// forward declarations
template <ComputeStage>
class ADMaterial;

template <>
InputParameters validParams<ADMaterial<RESIDUAL>>();
template <>
InputParameters validParams<ADMaterial<JACOBIAN>>();

/**
 * ADMaterials compute ADMaterialProperties.
 */
template <ComputeStage compute_stage>
class ADMaterial : public Material
{
public:
  ADMaterial(const InputParameters & parameters);
};

#endif // ADMATERIAL_H
