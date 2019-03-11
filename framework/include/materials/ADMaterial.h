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

#include "metaphysicl/numberarray.h"
#include "metaphysicl/dualnumber.h"

#define usingMaterialMembers                                                                       \
  usingCoupleableMembers;                                                                          \
  usingTransientInterfaceMembers;                                                                  \
  usingBlockRestrictableMembers;                                                                   \
  usingFunctionInterfaceMembers;                                                                   \
  using ConsoleStreamInterface::_console;                                                          \
  using ADMaterial<compute_stage>::_qp;                                                            \
  using ADMaterial<compute_stage>::_qrule;                                                         \
  using ADMaterial<compute_stage>::_JxW;                                                           \
  using ADMaterial<compute_stage>::_coord;                                                         \
  using ADMaterial<compute_stage>::_q_point;                                                       \
  using ADMaterial<compute_stage>::_current_elem;                                                  \
  using ADMaterial<compute_stage>::_fe_problem;                                                    \
  using ADMaterial<compute_stage>::_assembly;                                                      \
  using ADMaterial<compute_stage>::_mesh;                                                          \
  using ADMaterial<compute_stage>::isParamValid;                                                   \
  using ADMaterial<compute_stage>::paramError;                                                     \
  using ADMaterial<compute_stage>::copyDualNumbersToValues;                                        \
  using ADMaterial<compute_stage>::_displacements

// forward declarations
template <ComputeStage>
class ADMaterial;

declareADValidParams(ADMaterial);

/**
 * ADMaterials compute ADMaterialProperties.
 */
template <ComputeStage compute_stage>
class ADMaterial : public Material
{
public:
  ADMaterial(const InputParameters & parameters);

  /**
   * declare the ad property named "prop_name"
   */
  template <typename T>
  ADMaterialPropertyObject<T> & declareADProperty(const std::string & prop_name);
};

template <ComputeStage compute_stage>
template <typename T>
ADMaterialPropertyObject<T> &
ADMaterial<compute_stage>::declareADProperty(const std::string & prop_name)
{
  _fe_problem.usingADMatProps(true);
  registerPropName(prop_name, false, Material::CURRENT, compute_stage == JACOBIAN);
  return _material_data->declareADProperty<T>(prop_name);
}

#endif // ADMATERIAL_H
