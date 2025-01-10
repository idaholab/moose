//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolutionUserObject.h"

// MOOSE includes
#include "ConsoleUtils.h"
#include "MooseError.h"
#include "MooseMesh.h"
#include "MooseUtils.h"
#include "MooseVariableFE.h"
#include "RotationMatrix.h"
#include "Function.h"

// libMesh includes
#include "libmesh/equation_systems.h"
#include "libmesh/mesh_function.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/transient_system.h"
#include "libmesh/parallel_mesh.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/exodusII_io_helper.h"
#include "libmesh/enum_xdr_mode.h"

registerMooseObject("MooseApp", SolutionUserObject);

InputParameters
SolutionUserObject::validParams()
{
  // Get the input parameters from the parent class
  InputParameters params = SolutionUserObjectBase::validParams();
  params += FunctionParserUtils<false>::validParams();

  params.addCustomTypeParam<std::string>(
      "time_transformation",
      "t",
      "FunctionExpression",
      "Expression to transform from current simulation time to time at "
      "which to sample the solution.");

  return params;
}

SolutionUserObject::SolutionUserObject(const InputParameters & parameters)
  : SolutionUserObjectBase(parameters), FunctionParserUtils<false>(parameters)
{
  // setup parsed expression for the time transformation
  _time_transformation = std::make_shared<SymFunction>();
  setParserFeatureFlags(_time_transformation);

  // parse function
  const auto & expression = getParam<std::string>("time_transformation");
  if (_time_transformation->Parse(expression, "t") >= 0)
    mooseError("Invalid parsed function\n", expression, "\n", _time_transformation->ErrorMsg());

  // the only parameter is time
  _func_params.resize(1);
}

Real
SolutionUserObject::solutionSampleTime()
{
  _func_params[0] = _t;
  return evaluate(_time_transformation);
}
