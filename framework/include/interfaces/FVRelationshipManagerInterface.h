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
   */
  static void setRMParams(const InputParameters & obj_params,
                          InputParameters & rm_params,
                          const unsigned short ghost_layers,
                          const bool attach_geometric_early);

  /**
   * Helper function to set the relationship manager parameters
   * for advection-related kernels.
   * @param obj_params The input parameters of the object
   * @param rm_params The input parameters of the relationship manager
   * @param conditional_extended_layers Number of layers that will get assigned if the conditions in
   * the function are met
   * @param attach_geometric_early If the relationship manager should be attached early
   */
  static void setRMParamsAdvection(const InputParameters & obj_params,
                                   InputParameters & rm_params,
                                   const unsigned short conditional_extended_layers,
                                   const bool attach_geometric_early);

  /**
   * Helper function to set the relationship manager parameters
   * for diffusion-related kernels.
   * @param obj_params The input parameters of the object
   * @param rm_params The input parameters of the relationship manager
   * @param conditional_extended_layers Number of layers that will get assigned if the conditions in
   * the function are met
   * @param attach_geometric_early If the relationship manager should be attached early
   */
  static void setRMParamsDiffusion(const InputParameters & obj_params,
                                   InputParameters & rm_params,
                                   const unsigned short conditional_extended_layers,
                                   const bool attach_geometric_early);

  FVRelationshipManagerInterface() {}
};
