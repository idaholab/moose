//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UndisplacedMeshUpdater.h"
#include "libmesh/quadrature.h"

registerMooseObject("MooseApp", UndisplacedMeshUpdater);

InputParameters
UndisplacedMeshUpdater::validParams()
{
  InputParameters params = NodalUserObject::validParams();
  params.addClassDescription("Update nodal values for nodal variables on undisplaced mesh.");
  params.addRequiredCoupledVar(
      "variables", "Coupled variables that will be modified by the MeshModifier object.");
  params.addRequiredParam<Real>("threshold",
                                "The value above (or below) which to change the element subdomain");
  params.addParam<MooseEnum>("criterion_type",
                             MooseEnum("BELOW EQUAL ABOVE", "ABOVE"),
                             "Criterion to use for the threshold");
  params.addRequiredCoupledVar(
      "criterion_variable",
      "Coupled variable whose value is used in the criterion of activating the MeshModifier.");
  params.registerBase("MeshModifier");
  return params;
}

UndisplacedMeshUpdater::UndisplacedMeshUpdater(const InputParameters & parameters)
  : NodalUserObject(parameters),
    _n_vars(coupledComponents("variables")),
    _input_variables(coupledValues("variables")),
    _threshold(getParam<Real>("threshold")),
    _criterion_type(getParam<MooseEnum>("criterion_type").getEnum<CriterionType>()),
    _criterion_variable(coupledValue("criterion_variable"))
{
  for (unsigned int i = 0; i < _n_vars; i++)
    _output_variables.push_back(&writableVariable("variables", i));
}

void
UndisplacedMeshUpdater::execute()
{
  Real criterion = computeValue();
  switch (_criterion_type)
  {
    case CriterionType::Equal:
      if (!MooseUtils::absoluteFuzzyEqual(criterion - _threshold, 0))
        return;
      break;

    case CriterionType::Below:
      if (criterion >= _threshold)
        return;
      break;

    case CriterionType::Above:
      if (criterion <= _threshold)
        return;
      break;
  }

  const auto node_id = _current_node->id();
  auto & node = _mesh.nodeRef(node_id);
  for (unsigned int i = 0; i < _n_vars; i++)
  {
    auto current_value = (*_input_variables[i])[0];
    node(i) += current_value;
    _output_variables[i]->setNodalValue(0.0);
  }
}

void
UndisplacedMeshUpdater::finalize()
{
  // Reinit mesh, but notequation systems
  _fe_problem.meshChanged(
      /*intermediate_change=*/true, /*contract_mesh=*/false, /*clean_refinement_flags=*/false);
}

Real
UndisplacedMeshUpdater::computeValue()
{
  return _criterion_variable[0];
}
