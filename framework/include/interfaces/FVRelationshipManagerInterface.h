//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"
#include "MooseError.h"

/**
 * This class is responsible for adding relationship managers
 * that describe geometric, algebraic and coupling ghosting
 * for finite volume computations.
 */
class FVRelationshipManagerInterface
{
public:
  static InputParameters validParams();

  /**
   * Helper function to set the relationship manager parameters
   * for advection-related kernels.
   * @param obj_params The input parameters of the object
   * @param rm_params The input parameters of the relationship manager
   * @param conditional_extended_layers Number of layers that will get assigned if the conditions in
   * the function are met
   */
  static void setRMParamsAdvection(const InputParameters & obj_params,
                                   InputParameters & rm_params,
                                   const unsigned short conditional_extended_layers);

  /**
   * Helper function to set the relationship manager parameters
   * for diffusion-related kernels.
   * @param obj_params The input parameters of the object
   * @param rm_params The input parameters of the relationship manager
   * @param conditional_extended_layers Number of layers that will get assigned if the conditions in
   * the function are met
   */
  static void setRMParamsDiffusion(const InputParameters & obj_params,
                                   InputParameters & rm_params,
                                   const unsigned short conditional_extended_layers);

  FVRelationshipManagerInterface() {}

private:
  /**
   * Helper function to set the relationship manager parameters
   * @param obj_params The input parameters of the object
   * @param rm_params The input parameters of the relationship manager
   * @param ghost_layers The number of ghosted layers needed
   */
  static void setRMParams(const InputParameters & obj_params,
                          InputParameters & rm_params,
                          const unsigned short ghost_layers);

  /**
   * Throw an error with an acceptable context.
   * @param obj_params The object parameters
   * @param parameter_name The name of the parameter
   */
  template <typename T>
  static void parameterError(const InputParameters & obj_params,
                             const std::string & parameter_name,
                             const std::string & function_name,
                             const std::string & description);
};

template <typename T>
void
FVRelationshipManagerInterface::parameterError(const InputParameters & obj_params,
                                               const std::string & parameter_name,
                                               const std::string & function_name,
                                               const std::string & description)
{
  if (!obj_params.have_parameter<T>(parameter_name))
    mooseError(
        obj_params.blockFullpath(),
        " The following parameter ",
        parameter_name,
        " was not found! This is required for setting the correct amount of ghosting in parallel "
        "runs. This error is typically caused by utilizing incompatible relationship manager "
        "callback functions in different objects. An example can be using ",
        function_name,
        " in a(n)",
        description,
        " object.");
}
