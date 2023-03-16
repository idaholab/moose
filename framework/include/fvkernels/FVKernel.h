//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ResidualObject.h"
#include "BlockRestrictable.h"
#include "Assembly.h"
#include "ADFunctorInterface.h"

class SubProblem;

/// FVKernel is a base class for all finite volume method kernels.  Due to the
/// uniquely different needs for different types of finite volume kernels,
/// there is very little shared, common interface between them.  FV kernels are
/// broadly devided into two subgroups: numerical flux or surface integral
/// kernels (the FVFluxKernel class) and cell/volume integral kernels (the
/// FVElementalKernel class).  These FVKernels are stored in the moose app's
/// master "TheWarehouse" warehouse under the "FVFluxKernel" or
/// "FVElementalKernel" system names respectively.  FVKernels are generally
/// created by the CreateFVKernelsAction triggered by entries in the
/// "[FVKernels]" input file block.  FVKernels can only operate on and work
/// with finite volume variables (i.e. with the variable's parameter "fv = true" set).

class FVKernel : public ResidualObject, public BlockRestrictable, public ADFunctorInterface
{
public:
  static InputParameters validParams();
  static void setRMParams(const InputParameters & obj_params,
                          InputParameters & rm_params,
                          unsigned short ghost_layers);
  FVKernel(const InputParameters & params);
};
