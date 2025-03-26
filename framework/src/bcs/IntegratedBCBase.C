//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IntegratedBCBase.h"
#include "Assembly.h"

InputParameters
IntegratedBCBase::validParams()
{
  InputParameters params = BoundaryCondition::validParams();
  params += MaterialPropertyInterface::validParams();

  params.addParam<std::vector<AuxVariableName>>(
      "save_in",
      {},
      "The name of auxiliary variables to save this BC's residual contributions to.  "
      "Everything about that variable must match everything about this variable (the "
      "type, what blocks it's on, etc.)");
  params.addParam<std::vector<AuxVariableName>>(
      "diag_save_in",
      {},
      "The name of auxiliary variables to save this BC's diagonal jacobian "
      "contributions to.  Everything about that variable must match everything "
      "about this variable (the type, what blocks it's on, etc.)");

  params.addParamNamesToGroup("diag_save_in save_in", "Advanced");

  params.addParam<bool>(
      "skip_execution_outside_variable_domain",
      false,
      "Whether to skip execution of this boundary condition when the variable it "
      "applies to is not defined on the boundary. This can facilitate setups with "
      "moving variable domains and fixed boundaries. Note that the FEProblem boundary-restricted "
      "integrity checks will also need to be turned off if using this option");
  params.addParamNamesToGroup("skip_execution_outside_variable_domain", "Advanced");

  // Integrated BCs always rely on Boundary MaterialData
  params.set<Moose::MaterialDataType>("_material_data_type") = Moose::BOUNDARY_MATERIAL_DATA;

  return params;
}

IntegratedBCBase::IntegratedBCBase(const InputParameters & parameters)
  : BoundaryCondition(parameters, false), // False is because this is NOT nodal
    CoupleableMooseVariableDependencyIntermediateInterface(this, false),
    MaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, boundaryIDs()),
    _current_elem(_assembly.elem()),
    _current_elem_volume(_assembly.elemVolume()),
    _current_side(_assembly.side()),
    _current_side_elem(_assembly.sideElem()),
    _current_side_volume(_assembly.sideElemVolume()),
    _current_boundary_id(_assembly.currentBoundaryID()),
    _qrule(_assembly.qRuleFace()),
    _q_point(_assembly.qPointsFace()),
    _JxW(_assembly.JxWFace()),
    _coord(_assembly.coordTransformation()),
    _save_in_strings(parameters.get<std::vector<AuxVariableName>>("save_in")),
    _diag_save_in_strings(parameters.get<std::vector<AuxVariableName>>("diag_save_in")),
    _skip_execution_outside_variable_domain(
        getParam<bool>("skip_execution_outside_variable_domain"))
{
}

void
IntegratedBCBase::prepareShapes(const unsigned int var_num)
{
  _subproblem.prepareFaceShapes(var_num, _tid);
}

bool
IntegratedBCBase::shouldApply() const
{
#ifdef DEBUG
  const bool check_subdomain = true;
#else
  const bool check_subdomain = false;
#endif
  if (_skip_execution_outside_variable_domain || check_subdomain)
  {
    mooseAssert(_current_elem, "Should have a current element");
    const auto block_id = _current_elem->subdomain_id();
#ifdef DEBUG
    if (!_skip_execution_outside_variable_domain && !variable().hasBlocks(block_id))
      mooseError("This boundary condition is being executed outside the domain of "
                 "definition of its variable, on subdomain: ",
                 block_id);
#endif
    if (!variable().hasBlocks(block_id))
      return false;
  }
  return true;
}
