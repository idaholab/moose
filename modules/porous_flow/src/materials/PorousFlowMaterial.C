//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowMaterial.h"

#include "MaterialPropertyStorage.h"

#include "libmesh/quadrature.h"
#include "libmesh/fe_interface.h"

#include <limits>

InputParameters
PorousFlowMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  params.addParam<bool>(
      "at_nodes", false, "Evaluate Material properties at nodes instead of quadpoints");
  params.addPrivateParam<std::string>("pf_material_type", "pf_material");
  params.addClassDescription("This generalises MOOSE's Material class to allow for Materials that "
                             "hold information related to the nodes in the finite element");

  // Needed due to the custom tomfoolery going on with nodal material sizing in
  // initStatefulProperties()
  params.set<bool>("_force_stateful_init") = true;

  return params;
}

PorousFlowMaterial::PorousFlowMaterial(const InputParameters & parameters)
  : Material(parameters),
    _nodal_material(getParam<bool>("at_nodes")),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _pressure_variable_name("pressure_variable"),
    _saturation_variable_name("saturation_variable"),
    _temperature_variable_name("temperature_variable"),
    _mass_fraction_variable_name("mass_fraction_variable")
{
}

void
PorousFlowMaterial::initialSetup()
{
  if (!_nodal_material)
    return;

  // Tell the Dictator the FE type of every variable this Material reads at the
  // nodes, so that a single node count can be shared by all nodal Materials.
  // Elemental coupled variables are read by quadpoint instead (see the isNodal
  // guard in the derived classes) and so do not take part.
  for (const auto * const var : getCoupledMooseVars())
    if (var->isNodal())
      _dictator.registerNodalVariable(var->name());

  _material_data.onlyResizeIfSmaller(true);
  auto & storage = _material_data.getMaterialPropertyStorage();
  if (!storage.hasStatefulProperties())
    return;

  auto & stateful_prop_id_to_prop_id = storage.statefulProps();
  for (const auto i : index_range(stateful_prop_id_to_prop_id))
  {
    const auto prop_id = stateful_prop_id_to_prop_id[i];
    if (_supplied_prop_ids.count(prop_id))
      _supplied_old_prop_ids.push_back(i);
  }
}

void
PorousFlowMaterial::initStatefulProperties(unsigned int n_points)
{
  if (_nodal_material)
  {
    // size the properties to max(number_of_nodes, number_of_quadpoints)
    sizeNodalProperties();

    // compute the values for each node that carries a degree of freedom
    Material::initStatefulProperties(nodalDofCount());
  }
  else
    Material::initStatefulProperties(n_points);
}

void
PorousFlowMaterial::computeNodalProperties()
{
  const unsigned int numnodes = nodalDofCount();

  // compute the values for all nodes that carry a degree of freedom
  for (_qp = 0; _qp < numnodes; ++_qp)
    computeQpProperties();

  // If number_of_nodes < number_of_quadpoints, the remaining values in the
  // material data array are zero (for scalars) and empty (for vectors).
  // Unfortunately, this can cause issues with adaptivity, where the empty
  // value can be transferred to a node in a child element. This can lead
  // to a segfault when accessing stateful properties, see #14428.
  // To prevent this, we copy the last node value to the empty array positions.
  if (numnodes < _qrule->n_points())
  {
    MaterialProperties & props = _material_data.props();

    // Copy from qp = nodalDofCount() - 1 to qp = _qrule->n_points() - 1
    for (const auto & prop_id : _supplied_prop_ids)
      for (unsigned int qp = numnodes; qp < _qrule->n_points(); ++qp)
        props[prop_id].qpCopy(qp, props[prop_id], numnodes - 1);
  }
}

void
PorousFlowMaterial::computeProperties()
{
  if (_nodal_material)
  {
    // size the properties to max(number_of_nodes, number_of_quadpoints)
    sizeNodalProperties();

    computeNodalProperties();
  }
  else
    Material::computeProperties();
}

void
PorousFlowMaterial::sizeNodalProperties()
{
  /*
   * For nodal materials, the Properties should be sized as the maximum of
   * the number of nodes and the number of quadpoints.
   * We only actually need "number of nodes" pieces of information, which are
   * computed by computeProperties(), so the n_points - _current_elem->n_nodes()
   * elements at the end of the std::vector will always be zero, but they
   * are needed because MOOSE does copy operations (etc) that assumes that
   * the std::vector is sized to number of quadpoints.
   *
   * On boundary materials, the number of nodes may be larger than the number of
   * qps on the face of the element, in which case the remaining entries in the
   * material properties storage will be zero.
   *
   * \author lindsayad: MooseArray currently has the unfortunate side effect that if your new size
   * is greater than the current size, then we clear the whole data structure. Consequently this
   * call has the potential to clear material property evaluations done earlier in the material
   * dependency chain. So instead we selectively resize just our own properties and not everyone's
   */
  // _material_data.resize(std::max(_current_elem->n_nodes(), _qrule->n_points()));

  const auto new_size = std::max(_current_elem->n_nodes(), _qrule->n_points());
  auto & storage = _material_data.getMaterialPropertyStorage();

  auto & props = _material_data.props();
  for (const auto prop_id : _supplied_prop_ids)
    props[prop_id].resize(new_size);

  for (const auto state : storage.statefulIndexRange())
    for (const auto prop_id : _supplied_old_prop_ids)
      if (_material_data.props(state).hasValue(prop_id))
        _material_data.props(state)[prop_id].resize(new_size);
}

const VariableValue &
PorousFlowMaterial::nodalOrQpValue(const std::string & var_name, unsigned int comp)
{
  const bool is_nodal = isCoupled(var_name) ? getFieldVar(var_name, comp)->isNodal() : false;
  return (_nodal_material && is_nodal) ? coupledDofValues(var_name, comp)
                                       : coupledValue(var_name, comp);
}

unsigned int
PorousFlowMaterial::nodalDofCount() const
{
  // If no nodal Material reads any variable at the nodes then there is no short
  // array to overrun, and every node is visited as it always has been
  const auto & fe_type = _dictator.nodalFEType();
  if (!fe_type)
    return _current_elem->n_nodes();

  const auto num_dofs = libMesh::FEInterface::n_dofs(*fe_type, _current_elem);

  mooseAssert(num_dofs <= _current_elem->n_nodes(),
              "A nodal Material would visit " << num_dofs << " nodes of an element that has only "
                                              << _current_elem->n_nodes());

  // Everything here rests on libMesh numbering element nodes vertices-first and
  // numbering a LAGRANGE variable's degrees of freedom to match, so that degree
  // of freedom i lives at node i and looping 0 .. num_dofs - 1 visits exactly
  // the nodes that carry one.  That is a convention rather than something the
  // code enforces, and an unchecked convention of precisely this kind produced
  // the out-of-bounds read this count exists to prevent, so check it.
  mooseAssert(fe_type->family != libMesh::LAGRANGE || fe_type->order != libMesh::FIRST ||
                  num_dofs == _current_elem->n_vertices(),
              "First-order LAGRANGE has "
                  << num_dofs << " degrees of freedom on an element with "
                  << _current_elem->n_vertices()
                  << " vertices, so the assumption that they sit on the vertices, in order, does "
                     "not hold for this element type");

  return num_dofs;
}

unsigned
PorousFlowMaterial::nearestQP(unsigned nodenum) const
{
  unsigned nearest_qp = 0;
  Real smallest_dist = std::numeric_limits<Real>::max();
  for (const auto qp : make_range(_qrule->n_points()))
  {
    const Real this_dist = (_current_elem->point(nodenum) - _q_point[qp]).norm();
    if (this_dist < smallest_dist)
    {
      nearest_qp = qp;
      smallest_dist = this_dist;
    }
  }
  return nearest_qp;
}
