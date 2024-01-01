//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HybridizedKernel.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "SubProblem.h"
#include "NonlinearSystem.h"

#include "libmesh/threads.h"
#include "libmesh/quadrature.h"

InputParameters
HybridizedKernel::validParams()
{
  InputParameters params = KernelBase::validParams();
  params.registerBase("HybridizedKernel");
  params.suppressParameter<std::vector<AuxVariableName>>("save_in");
  params.suppressParameter<std::vector<AuxVariableName>>("diag_save_in");
  return params;
}

const std::string HybridizedKernel::lm_increment_vector_name = "hybrid_lm_increment";
