//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarPressureComponentAux.h"
#include "SystemBase.h"

registerMooseObject("ContactApp", MortarPressureComponentAux);

InputParameters
MortarPressureComponentAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;

  params.addClassDescription(
      "This class transforms the Cartesian Lagrange multiplier vector to local coordinates and "
      "outputs each individual component along the normal or tangential direction.");
  params.addRequiredCoupledVar("lm_var_x", "Lagrange multiplier variable along the x direction.");
  params.addRequiredCoupledVar("lm_var_y", "Lagrange multiplier variable along the y direction.");
  params.addCoupledVar(
      "lm_var_z",
      "Lagrange multiplier variable along the z direction (only exist for 3D problems).");
  params.addRequiredParam<MooseEnum>("component",
                                     MooseEnum("normal tangent1 tangent2"),
                                     "The component of the Lagrange multiplier to compute.");
  params.addRequiredParam<BoundaryName>("primary_boundary",
                                        "The name of the primary boundary sideset.");
  params.addRequiredParam<BoundaryName>("secondary_boundary",
                                        "The name of the secondary boundary sideset.");
  params.addParam<bool>(
      "use_displaced_mesh", true, "Whether to use the displaced mesh to get the mortar interface.");
  return params;
}

MortarPressureComponentAux::MortarPressureComponentAux(const InputParameters & params)
  : AuxKernel(params),
    _lm_var_x(&coupledValueLower("lm_var_x")),
    _lm_var_y(&coupledValueLower("lm_var_y")),
    _lm_var_z(params.isParamValid("lm_var_z") ? &coupledValueLower("lm_var_z") : nullptr),
    _fe_problem(*params.get<FEProblemBase *>("_fe_problem_base")),
    _primary_id(_fe_problem.mesh().getBoundaryID(getParam<BoundaryName>("primary_boundary"))),
    _secondary_id(_fe_problem.mesh().getBoundaryID(getParam<BoundaryName>("secondary_boundary"))),
    _component(getParam<MooseEnum>("component").getEnum<ComponentType>()),
    _use_displaced_mesh(getParam<bool>("use_displaced_mesh"))
{
  // Only consider nodal quantities
  if (!isNodal())
    mooseError("MortarPressureComponent auxiliary kernel can only be used with nodal kernels.");

  if (!_use_displaced_mesh)
    paramError("use_displaced_mesh",
               "This auxiliary kernel requires the use of displaced meshes to compute the "
               "frictional pressure vector.");

  // Kernel need to be boundary restricted
  if (!this->_bnd)
    paramError("boundary",
               "MortarPressureComponent auxiliary kernel must be restricted to a boundary.");

  // Get mortar interfaces
  const auto & displaced_mortar_interfaces =
      _fe_problem.getMortarInterfaces(/*displaced=*/_use_displaced_mesh);

  if (displaced_mortar_interfaces.size() == 0)
    paramError("lm_var_x",
               "No mortar interface could be identified in this problem. Make sure mortar contact "
               "is enabled.");

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
MortarPressureComponentAux::computeValue()
{
  // A node id may correspond to more than one lower-d elements on the secondary surface.
  // However, we are looping over nodes below, so we will locate the correct geometry
  const Elem * lower_dimensional_element =
      libmesh_map_find(_mortar_generation_object->nodesToSecondaryElem(), _current_node->id())[0];

  // Get the nodal normals for this element
  const auto & nodal_normals =
      _mortar_generation_object->getNodalNormals(*lower_dimensional_element);

  // Get nodal tangents for this element
  const auto & nodal_tangents =
      _mortar_generation_object->getNodalTangents(*lower_dimensional_element);

  Point normal_vector, tangent1, tangent2;

  for (const auto lowerd_node : make_range(lower_dimensional_element->n_nodes()))
    if (_current_node->id() == lower_dimensional_element->node_id(lowerd_node))
    {
      normal_vector = nodal_normals[lowerd_node];
      tangent1 = nodal_tangents[0][lowerd_node];
      tangent2 = nodal_tangents[1][lowerd_node];
      break;
    }

  Point lm_vector_value(
      (*_lm_var_x)[_qp], (*_lm_var_y)[_qp], _lm_var_z == nullptr ? 0.0 : (*_lm_var_z)[_qp]);

  Real pressure_component_value = 0.0;

  switch (_component)
  {
    case ComponentType::NORMAL:
      pressure_component_value = lm_vector_value * normal_vector;
      break;

    case ComponentType::TANGENT1:
      pressure_component_value = lm_vector_value * tangent1;
      break;

    case ComponentType::TANGENT2:
      pressure_component_value = lm_vector_value * tangent2;
      break;
  }

  return pressure_component_value;
}
