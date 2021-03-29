//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarKernel.h"
#include "Assembly.h"
#include "MooseVariableScalar.h"
#include "MooseVariableFE.h"
#include "Problem.h"
#include "SubProblem.h"
#include "NonlinearSystem.h"

defineLegacyParams(ScalarKernel);

InputParameters
ScalarKernel::validParams()
{
  InputParameters params = ResidualObject::validParams();
  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation.  Note that "
                        "in the case this is true but no "
                        "displacements are provided in the Mesh block "
                        "the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  params.registerBase("ScalarKernel");

  return params;
}

ScalarKernel::ScalarKernel(const InputParameters & parameters)
  : ResidualObject(parameters),
    ScalarCoupleable(this),
    _var(_sys.getScalarVariable(_tid, parameters.get<NonlinearVariableName>("variable"))),
    _u(_is_implicit ? _var.sln() : uOld())
{
}

const VariableValue &
ScalarKernel::uOld() const
{
  if (_sys.solutionStatesInitialized())
    mooseError("The solution states have already been initialized when calling ",
               type(),
               "::uOld().\n\n",
               "Make sure to call uOld() within the object constructor.");

  return _var.slnOld();
}
