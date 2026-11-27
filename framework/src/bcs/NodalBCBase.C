//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalBCBase.h"

InputParameters
NodalBCBase::validParams()
{
  InputParameters params = BoundaryCondition::validParams();
  params.addDeprecatedParam<std::vector<AuxVariableName>>(
      "save_in",
      {},
      "The name of auxiliary variables to save this BC's residual contributions to.  "
      "Everything about that variable must match everything about this variable (the "
      "type, what blocks it's on, etc.)",
      "Add this object's residual to a tagged vector with 'extra_vector_tags', then transfer that "
      "vector into an auxiliary variable with a TagVectorAux");
  params.addDeprecatedParam<std::vector<AuxVariableName>>(
      "diag_save_in",
      {},
      "The name of auxiliary variables to save this BC's diagonal jacobian "
      "contributions to.  Everything about that variable must match everything "
      "about this variable (the type, what blocks it's on, etc.)",
      "Add this object's Jacobian to a tagged matrix with 'extra_matrix_tags', then transfer that "
      "matrix diagonal into an auxiliary variable with a TagMatrixAux");

  params.addParamNamesToGroup("vector_tags matrix_tags", "Tagging");

  params.set<MultiMooseEnum>("vector_tags") = "residual";
  params.set<MultiMooseEnum>("matrix_tags") = "system time";
  return params;
}

NodalBCBase::NodalBCBase(const InputParameters & parameters)
  : BoundaryCondition(parameters, true), // true is for being Nodal
    CoupleableMooseVariableDependencyIntermediateInterface(this, true),
    _save_in_strings(parameters.get<std::vector<AuxVariableName>>("save_in")),
    _diag_save_in_strings(parameters.get<std::vector<AuxVariableName>>("diag_save_in"))
{
}
