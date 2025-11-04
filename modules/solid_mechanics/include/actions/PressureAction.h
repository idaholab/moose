//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

/**
 * Macro to create an action for a PressureBase derived pressure boundary condition.
 *  - use declareMoosePressureAction in the header file of your pressure bc
 *  - use registerMoosePressureAction in the source file of your pressure bc
 */
#define stringifyName(name) #name
#define registerMoosePressureAction(appname, bc_class, action_class)                               \
  action_class::action_class(const InputParameters & params)                                       \
    : PressureActionBase(params, stringifyName(bc_class), "AD" stringifyName(bc_class))            \
  {                                                                                                \
  }                                                                                                \
  InputParameters action_class::validParams()                                                      \
  {                                                                                                \
    auto params = PressureActionBase::validParams<bc_class>();                                     \
    params.addClassDescription(                                                                    \
        "Set up pressure boundary condition using the " stringifyName(bc_class) " object.");       \
    return params;                                                                                 \
  }                                                                                                \
  registerMooseAction(appname, action_class, "add_bc")

#define declareMoosePressureAction(bc_class, action_class)                                         \
  class action_class : public PressureActionBase                                                   \
  {                                                                                                \
  public:                                                                                          \
    action_class(const InputParameters & params);                                                  \
    static InputParameters validParams();                                                          \
  }

/**
 * Pressure boundary condition action template. The type T is used to get the
 * valid parameters through T::actionParams(). The two strings passed into the constructor are the
 * registred type names of teh MOOSE BC objects without and with automatic differentiation.
 */
class PressureActionBase : public Action
{
public:
  template <class T>
  static InputParameters validParams();

  PressureActionBase(const InputParameters & params,
                     const std::string & non_ad_pressure_bc_type,
                     const std::string & ad_pressure_bc_type);

  virtual void act() override;

protected:
  const std::string _non_ad_pressure_bc_type;
  const std::string _ad_pressure_bc_type;

  /// Flag to use automatic differentiation
  const bool _use_ad;

  const std::vector<std::vector<AuxVariableName>> _save_in_vars;
  const std::vector<bool> _has_save_in_vars;
};

template <class T>
InputParameters
PressureActionBase::validParams()
{
  InputParameters params = Action::validParams();
  params += T::actionParams();

  params.addParam<std::vector<BoundaryName>>(
      "boundary", "The list of boundaries (ids or names) from the mesh where this object applies");

  params.addClassDescription("Set up Pressure boundary conditions");
  params.addParam<bool>("use_automatic_differentiation",
                        false,
                        "Flag to use automatic differentiation (AD) objects when possible");

  // To make controlling the Pressure BCs easier
  params.addParam<bool>(
      "enable",
      true,
      "Set the enabled status of the BCs created by the Pressure action (defaults to true).");
  params.declareControllable("enable");

  // Residual output
  params.addParam<std::vector<AuxVariableName>>(
      "save_in_disp_x", {}, "The save_in variables for x displacement");
  params.addParam<std::vector<AuxVariableName>>(
      "save_in_disp_y", {}, "The save_in variables for y displacement");
  params.addParam<std::vector<AuxVariableName>>(
      "save_in_disp_z", {}, "The save_in variables for z displacement");
  params.addParam<std::vector<TagName>>("extra_vector_tags",
                                        "The extra tags for the vectors this Kernel should fill");
  params.addParam<std::vector<TagName>>(
      "absolute_value_vector_tags",
      "The tags for the vectors this residual object should fill with the "
      "absolute value of the residual contribution");

  params.addParam<bool>(
      "use_displaced_mesh",
      "Whether or not this object should use the displaced mesh for computation. Note that in "
      "the case this is true but no displacements are provided in the Mesh block the undisplaced "
      "mesh will still be used. For small strain formulations pressure should be applied to the "
      "undisplaced mesh to obtain agreement with analytical benchmark solutions.");

  params.addParamNamesToGroup(
      "save_in_disp_x save_in_disp_y save_in_disp_z extra_vector_tags absolute_value_vector_tags",
      "Residual output");
  return params;
}
