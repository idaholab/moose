//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarNodalGapAux.h"
#include "SystemBase.h"

registerMooseObject("ContactApp", MortarNodalGapAux);

InputParameters
MortarNodalGapAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.set<ExecFlagEnum>("execute_on") = EXEC_NONLINEAR;

  params.addClassDescription("This class creates an auxiliary vector for outputting the mortar "
                             "frictional pressure vector.");
  params.addRequiredCoupledVar(
      "tangent_one",
      "First tangent vector Lagrange's multiplier for computing the mortar "
      "frictional pressure vector.");
  params.addRequiredCoupledVar(
      "tangent_two",
      "Second tangent vector Lagrange's multiplier for computing the mortar "
      "frictional pressure vector.");
  params.addRequiredParam<unsigned int>("component",
                                        "Cartesian component of frictional tangent vector");

  params.addRequiredParam<BoundaryName>("primary_boundary",
                                        "The name of the primary boundary sideset.");
  params.addRequiredParam<BoundaryName>("secondary_boundary",
                                        "The name of the secondary boundary sideset.");
  params.addParam<bool>(
      "use_displaced_mesh", true, "Whether to use the displaced mesh to get the mortar interface.");
  return params;
}

MortarNodalGapAux::MortarNodalGapAux(const InputParameters & params)
  : AuxKernel(params),
    _tangent_one(&coupledValueLower("tangent_one")),
    _tangent_two(&coupledValueLower("tangent_two")),
    _fe_problem(*params.get<FEProblemBase *>("_fe_problem_base")),
    _primary_id(_fe_problem.mesh().getBoundaryID(getParam<BoundaryName>("primary_boundary"))),
    _secondary_id(_fe_problem.mesh().getBoundaryID(getParam<BoundaryName>("secondary_boundary"))),
    _component(getParam<unsigned int>("component")),
    _use_displaced_mesh(getParam<bool>("use_displaced_mesh"))
{
  // Only consider nodal quantities
  if (!isNodal())
    mooseError(
        "MortarFrictionalPressureVector auxiliary kernel can only be used with nodal kernels.");

  if (!_use_displaced_mesh)
    paramError("use_displaced_mesh",
               "This auxiliary kernel requires the use of displaced meshes to compute the "
               "frictional pressure vector.");

  // Kernel need to be boundary restricted
  if (!this->_bnd)
    paramError("boundary",
               "MortarFrictionalPressureVector auxiliary kernel must be restricted to a boundary.");

  // Get mortar interfaces
  const auto & displaced_mortar_interfaces =
      _fe_problem.getMortarInterfaces(/*displaced=*/_use_displaced_mesh);

  if (displaced_mortar_interfaces.size() == 0)
    paramError("tangent_one",
               "No mortar interface could be identified in this problem. Make sure mortar contact "
               "is enabled.");

  const auto mortar_dimension = _fe_problem.mesh().dimension() - 1;
  if (mortar_dimension == 1)
    paramError("tangent_two",
               "MortarFrictionalPressureVector auxiliary kernel can only be used in "
               "three-dimensional problems");

  // Get automatic generation object for the boundary pair this auxiliary acts on.
  if (displaced_mortar_interfaces.count(std::make_pair(_primary_id, _secondary_id)) != 1)
    mooseError("primary_boundary",
               "The boundary pairs do not correspond to a single mortar contact boundary pair. "
               "Please revise your input file for proper mortar contact constraints and mortar "
               "frictional pressure vector auxiliary variable definition.");

  _mortar_generation_object =
      &libmesh_map_find(displaced_mortar_interfaces, std::make_pair(_primary_id, _secondary_id));
}

Real
MortarNodalGapAux::computeValue()
{

  // Note: Nodal kernels cannot make use of _current_elem
  const std::map<dof_id_type, std::vector<dof_id_type>> & node_to_elem_map = _mesh.nodeToElemMap();
  auto node_to_elem_pair = node_to_elem_map.find(_current_node->id());
  mooseAssert(node_to_elem_pair != node_to_elem_map.end(), "Missing entry in node to elem map");
  std::vector<dof_id_type> element_ids = node_to_elem_pair->second;

  // We can pick any element id since we are looping over nodes below.
  const Elem * lower_dimensional_element =
      _mortar_generation_object->getSecondaryLowerdElemFromSecondaryElem(element_ids[0]);

  // Get nodal tangents for this element
  std::array<std::vector<Point>, 2> nodal_tangents =
      _mortar_generation_object->getNodalTangents(*lower_dimensional_element);

  Real tangent_one_component = 0;
  Real tangent_two_component = 0;

  for (const auto lowerd_node : make_range(lower_dimensional_element->n_nodes()))
  {
    if (_fe_problem.mesh().getMesh().node_ptr(_current_node->id()) ==
        _fe_problem.mesh().getMesh().node_ptr(lower_dimensional_element->node_id(lowerd_node)))
    {
      tangent_one_component = nodal_tangents[0][lowerd_node](_component);
      tangent_two_component = nodal_tangents[1][lowerd_node](_component);
      break;
    }
  }

  // Compute nodal tangent vector component
  const Real tangent_vector_component =
      tangent_one_component * (*_tangent_one)[_qp] + tangent_two_component * (*_tangent_two)[_qp];

  return tangent_vector_component;
}
