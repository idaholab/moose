//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowMaterial.h"
#include "libmesh/quadrature.h"
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

  _material_data->onlyResizeIfSmaller(true);
  auto & storage = _material_data->getMaterialPropertyStorage();
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

    // compute the values for number_of_nodes
    Material::initStatefulProperties(_current_elem->n_nodes());
  }
  else
    Material::initStatefulProperties(n_points);
}

void
PorousFlowMaterial::computeNodalProperties()
{
  const unsigned int numnodes = _current_elem->n_nodes();

  // compute the values for all nodes
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
    MaterialProperties & props = _material_data->props();

    // Copy from qp = _current_elem->n_nodes() - 1 to qp = _qrule->n_points() -1
    for (const auto & prop_id : _supplied_prop_ids)
      for (unsigned int qp = numnodes; qp < _qrule->n_points(); ++qp)
        props[prop_id]->qpCopy(qp, props[prop_id], numnodes - 1);
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
  // _material_data->resize(std::max(_current_elem->n_nodes(), _qrule->n_points()));

  const auto new_size = std::max(_current_elem->n_nodes(), _qrule->n_points());
  auto & storage = _material_data->getMaterialPropertyStorage();
  auto & props = _material_data->props();
  auto & props_old = _material_data->propsOld();
  auto & props_older = _material_data->propsOlder();

  for (const auto prop_id : _supplied_prop_ids)
    props[prop_id]->resize(new_size);

  for (const auto prop_id : _supplied_old_prop_ids)
    if (auto * const old_prop = props_old[prop_id])
      old_prop->resize(new_size);

  if (storage.hasOlderProperties())
    for (const auto prop_id : _supplied_old_prop_ids)
      if (auto * const older_prop = props_older[prop_id])
        older_prop->resize(new_size);
}

unsigned
PorousFlowMaterial::nearestQP(unsigned nodenum) const
{
  unsigned nearest_qp = 0;
  Real smallest_dist = std::numeric_limits<Real>::max();
  for (unsigned qp = 1; qp < _qrule->n_points(); ++qp)
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
