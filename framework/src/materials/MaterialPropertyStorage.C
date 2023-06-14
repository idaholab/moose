//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialPropertyStorage.h"
#include "MaterialProperty.h"
#include "Material.h"
#include "MaterialData.h"
#include "MooseMesh.h"

#include "libmesh/fe_interface.h"
#include "libmesh/quadrature.h"

MaterialPropertyStorage::MaterialPropertyStorage(MaterialPropertyStorage::Registry & registry)
  : _max_state(0), _spin_mtx(libMesh::Threads::spin_mtx), _registry(registry)
{
}

void
MaterialPropertyStorage::shallowSwapData(const std::vector<unsigned int> & stateful_prop_ids,
                                         MaterialProperties & data,
                                         MaterialProperties & data_from)
{
  for (const auto i : make_range(std::min(stateful_prop_ids.size(), data_from.size())))
    if (stateful_prop_ids[i] < data.size() && data.hasValue(stateful_prop_ids[i]) &&
        data_from.hasValue(i))
    {
      auto & prop = data[stateful_prop_ids[i]];
      auto & prop_from = data_from[i];
      prop.swap(prop_from);
    }
}

void
MaterialPropertyStorage::shallowSwapDataBack(const std::vector<unsigned int> & stateful_prop_ids,
                                             MaterialProperties & data,
                                             MaterialProperties & data_from)
{
  for (const auto i : make_range(std::min(stateful_prop_ids.size(), data.size())))
    if (stateful_prop_ids[i] < data_from.size() && data.hasValue(i) &&
        data_from.hasValue(stateful_prop_ids[i]))
    {
      auto & prop = data[i];
      auto & prop_from = data_from[stateful_prop_ids[i]];
      prop.swap(prop_from);
    }
}

void
MaterialPropertyStorage::eraseProperty(const Elem * elem)
{
  for (const auto state : stateIndexRange())
    setProps(state).erase(elem);
}

void
MaterialPropertyStorage::prolongStatefulProps(
    processor_id_type pid,
    const std::vector<std::vector<QpMap>> & refinement_map,
    const QBase & qrule,
    const QBase & qrule_face,
    MaterialPropertyStorage & parent_material_props,
    MaterialData & child_material_data,
    const Elem & elem,
    const int input_parent_side,
    const int input_child,
    const int input_child_side)
{
  mooseAssert(input_child != -1 || input_parent_side == input_child_side, "Invalid inputs!");

  unsigned int n_qpoints = 0;

  // If we passed in -1 for these then we really need to store properties at 0
  unsigned int parent_side = input_parent_side == -1 ? 0 : input_parent_side;
  unsigned int child_side = input_child_side == -1 ? 0 : input_child_side;

  if (input_child_side == -1) // Not doing side projection (ie, doing volume projection)
    n_qpoints = qrule.n_points();
  else
    n_qpoints = qrule_face.n_points();

  child_material_data.resize(n_qpoints);

  unsigned int n_children = elem.n_children();

  std::vector<unsigned int> children;

  if (input_child != -1) // Passed in a child explicitly
    children.push_back(input_child);
  else
  {
    children.resize(n_children);
    for (unsigned int child = 0; child < n_children; child++)
      children[child] = child;
  }

  for (const auto & child : children)
  {
    // If we're not projecting an internal child side, but we are projecting sides, see if this
    // child is on that side
    if (input_child == -1 && input_child_side != -1 && !elem.is_child_on_side(child, parent_side))
      continue;

    const Elem * child_elem = elem.child_ptr(child);

    // If it's not a local child then it'll be prolonged where it is
    // local
    if (child_elem->processor_id() != pid)
      continue;

    mooseAssert(child < refinement_map.size(), "Refinement_map vector not initialized");
    const std::vector<QpMap> & child_map = refinement_map[child];

    initProps(child_material_data, child_elem, child_side, n_qpoints);

    for (const auto state : stateIndexRange())
    {
      const auto & parent_props = parent_material_props.props(&elem, parent_side, state);
      auto & child_props = setProps(child_elem, child_side, state);
      for (const auto i : index_range(_stateful_prop_id_to_prop_id))
        for (const auto qp : index_range(refinement_map[child]))
          child_props[i].qpCopy(qp, parent_props[i], child_map[qp]._to);
    }
  }
}

void
MaterialPropertyStorage::restrictStatefulProps(
    const std::vector<std::pair<unsigned int, QpMap>> & coarsening_map,
    const std::vector<const Elem *> & coarsened_element_children,
    const QBase & qrule,
    const QBase & qrule_face,
    MaterialData & material_data,
    const Elem & elem,
    int input_side)
{
  unsigned int side;

  bool doing_a_side = input_side != -1;

  unsigned int n_qpoints = 0;

  if (!doing_a_side)
  {
    side = 0; // Use 0 for the elem
    n_qpoints = qrule.n_points();
  }
  else
  {
    side = input_side;
    n_qpoints = qrule_face.n_points();
  }

  initProps(material_data, &elem, side, n_qpoints);

  std::vector<MaterialProperties *> parent_props(numStates());
  for (const auto state : stateIndexRange())
    parent_props[state] = &setProps(&elem, side, state);

  // Copy from the child stateful properties
  for (const auto qp : index_range(coarsening_map))
  {
    const std::pair<unsigned int, QpMap> & qp_pair = coarsening_map[qp];
    unsigned int child = qp_pair.first;

    mooseAssert(child < coarsened_element_children.size(),
                "Coarsened element children vector not initialized");
    const Elem * child_elem = coarsened_element_children[child];
    const QpMap & qp_map = qp_pair.second;

    for (const auto state : stateIndexRange())
    {
      const auto & child_props = props(child_elem, side, state);
      for (const auto i : index_range(_stateful_prop_id_to_prop_id))
        (*parent_props[state])[i].qpCopy(qp, child_props[i], qp_map._to);
    }
  }
}

void
MaterialPropertyStorage::initStatefulProps(MaterialData & material_data,
                                           const std::vector<std::shared_ptr<MaterialBase>> & mats,
                                           unsigned int n_qpoints,
                                           const Elem & elem,
                                           unsigned int side /* = 0*/)
{
  // NOTE: since materials are storing their computed properties in MaterialData class, we need to
  // juggle the memory between MaterialData and MaterialProperyStorage classes

  initProps(material_data, &elem, side, n_qpoints);

  // copy from storage to material data
  swap(material_data, elem, side);
  // run custom init on properties
  for (const auto & mat : mats)
    mat->initStatefulProperties(n_qpoints);

  swapBack(material_data, elem, side);

  if (!hasStatefulProperties())
    return;

  // This second call to initProps covers cases where code in
  // "init[Qp]StatefulProperties" may have called a get/declare for a stateful
  // property affecting the _stateful_prop_id_to_prop_id vector among other
  // things.  This is necessary because a call to
  // getMaterialProperty[Old/Older] can potentially trigger a material to
  // become stateful that previously wasn't.  This needs to go after the
  // swapBack.
  initProps(material_data, &elem, side, n_qpoints);

  // Copy to older states as needed
  const auto & current_props = props(&elem, side, 0);
  for (const auto state : statefulIndexRange())
  {
    auto & to_props = setProps(&elem, side, state);
    for (const auto i : index_range(_stateful_prop_id_to_prop_id))
      for (const auto qp : make_range(n_qpoints))
        to_props[i].qpCopy(qp, current_props[i], qp);
  }
}

void
MaterialPropertyStorage::shift()
{
  mooseAssert(hasStatefulProperties(), "Doesn't have stateful props");

  /**
   * Shift properties back in time and reuse older data for current (save reallocations etc.)
   * With current, old, and older this can be accomplished by two swaps:
   * older <-> old
   * old <-> current
   */
  for (unsigned int state = maxState(); state != 0; state--)
    std::swap(setProps(state), setProps(state - 1));
}

void
MaterialPropertyStorage::copy(MaterialData & material_data,
                              const Elem & elem_to,
                              const Elem & elem_from,
                              unsigned int side,
                              unsigned int n_qpoints)
{
  copy(material_data, &elem_to, &elem_from, side, n_qpoints);
}

void
MaterialPropertyStorage::copy(MaterialData & material_data,
                              const Elem * elem_to,
                              const Elem * elem_from,
                              unsigned int side,
                              unsigned int n_qpoints)
{
  initProps(material_data, elem_to, side, n_qpoints);

  for (const auto state : stateIndexRange())
  {
    const auto & from_props = props(elem_from, side, state);
    auto & to_props = setProps(elem_to, side, state);
    for (const auto i : index_range(_stateful_prop_id_to_prop_id))
      for (const auto qp : make_range(n_qpoints))
        to_props[i].qpCopy(qp, from_props[i], qp);
  }
}

void
MaterialPropertyStorage::swap(MaterialData & material_data, const Elem & elem, unsigned int side)
{
  Threads::spin_mutex::scoped_lock lock(this->_spin_mtx);

  for (const auto state : stateIndexRange())
    shallowSwapData(_stateful_prop_id_to_prop_id,
                    material_data.props(state),
                    // Would be nice to make this setProps()
                    initAndSetProps(&elem, side, state));
}

void
MaterialPropertyStorage::swapBack(MaterialData & material_data,
                                  const Elem & elem,
                                  unsigned int side)
{
  Threads::spin_mutex::scoped_lock lock(this->_spin_mtx);

  for (const auto state : stateIndexRange())
    shallowSwapDataBack(
        _stateful_prop_id_to_prop_id, setProps(&elem, side, state), material_data.props(state));

  // Workaround for MOOSE difficulties in keeping materialless
  // elements (e.g. Lower D elements in Mortar code) materials
  for (const auto state : stateIndexRange())
    if (props(&elem, side, state).empty())
      setProps(state)[&elem].erase(side);
}

bool
MaterialPropertyStorage::hasProperty(const std::string & prop_name) const
{
  return _registry.prop_ids.count(prop_name) > 0;
}

unsigned int
MaterialPropertyStorage::addProperty(const std::string & prop_name, const unsigned int state)
{
  if (state > max_state)
    mooseError("Material property state of ",
               state,
               " is not supported. Max state supported: ",
               max_state);

  // Increment state as needed
  if (maxState() < state)
    _max_state = state;

  const auto prop_id = getPropertyId(prop_name);

  if (state > 0)
  {
    if (std::find(_stateful_prop_id_to_prop_id.begin(),
                  _stateful_prop_id_to_prop_id.end(),
                  prop_id) == _stateful_prop_id_to_prop_id.end())
      _stateful_prop_id_to_prop_id.push_back(prop_id);
    _stateful_prop_names[prop_id] = prop_name;
  }

  return prop_id;
}

unsigned int
MaterialPropertyStorage::getPropertyId(const std::string & prop_name)
{
  const auto it = _registry.prop_ids.find(prop_name);
  if (it != _registry.prop_ids.end())
    return it->second;

  const auto id = _registry.prop_names.size();
  _registry.prop_names.push_back(prop_name);
  _registry.prop_ids.emplace(prop_name, id);
  return id;
}

unsigned int
MaterialPropertyStorage::retrievePropertyId(const std::string & prop_name) const
{
  auto it = _registry.prop_ids.find(prop_name);
  if (it == _registry.prop_ids.end())
    mooseError("MaterialPropertyStorage: property " + prop_name + " is not yet declared");
  return it->second;
}

void
MaterialPropertyStorage::initProps(MaterialData & material_data,
                                   const Elem * elem,
                                   unsigned int side,
                                   unsigned int n_qpoints)
{
  for (const auto state : stateIndexRange())
    this->initProps(material_data, state, elem, side, n_qpoints);
}

void
MaterialPropertyStorage::initProps(MaterialData & material_data,
                                   const unsigned int state,
                                   const Elem * elem,
                                   unsigned int side,
                                   unsigned int n_qpoints)
{
  material_data.resize(n_qpoints);

  auto & mat_props = initAndSetProps(elem, side, state);

  // In some special cases, material_data might be larger than n_qpoints
  if (material_data.isOnlyResizeIfSmaller())
    n_qpoints = material_data.nQPoints();

  const auto n_props = _stateful_prop_id_to_prop_id.size();
  if (mat_props.size() < n_props)
    mat_props.resize(n_props, {});

  // init properties (allocate memory. etc)
  for (const auto i : index_range(_stateful_prop_id_to_prop_id))
    if (!mat_props.hasValue(i))
    {
      const auto prop_id = _stateful_prop_id_to_prop_id[i];
      mat_props.setValue(i, {}) = material_data.props(0)[prop_id].clone(n_qpoints);
    }
}

void
dataStore(std::ostream & stream, MaterialPropertyStorage & storage, void * context)
{
  unsigned int max_state = storage.maxState();
  dataStore(stream, max_state, context);

  for (const auto state : storage.stateIndexRange())
    dataStore(stream, storage.setProps(state), context);
}

void
dataLoad(std::istream & stream, MaterialPropertyStorage & storage, void * context)
{
  unsigned int state_index;
  dataLoad(stream, state_index, context);
  if (state_index != storage.maxState())
    mooseError("Inconsistent max state; stored max state = ",
               state_index,
               ", current max state = ",
               storage.maxState());

  for (const auto state : storage.stateIndexRange())
    dataLoad(stream, storage.setProps(state), context);
}
