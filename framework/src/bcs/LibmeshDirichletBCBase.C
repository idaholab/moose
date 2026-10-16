//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LibmeshDirichletBCBase.h"

#include "MooseVariableFE.h"

#include "libmesh/fe_interface.h"

const std::string LibmeshDirichletBCBase::marker_param = "_libmesh_dirichlet_bc";

InputParameters
LibmeshDirichletBCBase::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += BoundaryRestrictableRequired::validParams();

  params.addRequiredParam<NonlinearVariableName>(
      "variable", "The name of the variable this boundary condition constrains");

  // libMesh owns the constraint once it is registered, and adding or removing one changes which
  // degrees of freedom are unknowns, so these objects cannot be switched on and off
  params.suppressParameter<bool>("enable");

  // Tells FEProblemBase::addBoundaryCondition() to register this with libMesh rather than as a
  // residual object
  params.addPrivateParam<bool>(LibmeshDirichletBCBase::marker_param, true);

  params.registerBase("BoundaryCondition");
  params.registerSystemAttributeName("BoundaryCondition");
  return params;
}

LibmeshDirichletBCBase::LibmeshDirichletBCBase(const InputParameters & parameters)
  : MooseObject(parameters),
    BoundaryRestrictableRequired(this, /*nodal=*/false),
    FunctionInterface(this),
    MooseVariableInterface<Real>(this,
                                 /*nodal=*/false,
                                 "variable",
                                 Moose::VarKindType::VAR_SOLVER,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _var(*mooseVariable())
{
  // A C1 family's projection needs a gradient functor that a MOOSE boundary condition has no way
  // to supply; libMesh's ConstrainDirichlet only calls a plain-valued FunctionBase for it
  if (libMesh::FEInterface::get_continuity(_var.feType()) == libMesh::C_ONE)
    paramError("variable",
               "This boundary condition is imposed by libMesh's Dirichlet constraint machinery, "
               "which needs the gradient of the prescribed value to project it onto a variable "
               "whose finite element family has C1 continuity. This is not supported.");
}
