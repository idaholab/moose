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

#include <optional>

MaterialPropertyStorage::MaterialPropertyStorage(MaterialPropertyRegistry & registry)
  : _max_state(0), _spin_mtx(libMesh::Threads::spin_mtx), _registry(registry)
{
  _material_data.reserve(libMesh::n_threads());
  for (const auto tid : make_range(libMesh::n_threads()))
    _material_data.emplace_back(*this, tid);
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
    const THREAD_ID tid,
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

  getMaterialData(tid).resize(n_qpoints);

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

    initProps(tid, child_elem, child_side, n_qpoints);

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
    const THREAD_ID tid,
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

  initProps(tid, &elem, side, n_qpoints);

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
MaterialPropertyStorage::initStatefulProps(const THREAD_ID tid,
                                           const std::vector<std::shared_ptr<MaterialBase>> & mats,
                                           const unsigned int n_qpoints,
                                           const Elem & elem,
                                           const unsigned int side /* = 0*/)
{
  // NOTE: since materials are storing their computed properties in MaterialData class, we need to
  // juggle the memory between MaterialData and MaterialProperyStorage classes

  initProps(tid, &elem, side, n_qpoints);

  // copy from storage to material data
  swap(tid, elem, side);
  // run custom init on properties
  for (const auto & mat : mats)
    mat->initStatefulProperties(n_qpoints);

  swapBack(tid, elem, side);

  if (!hasStatefulProperties())
    return;

  // This second call to initProps covers cases where code in
  // "init[Qp]StatefulProperties" may have called a get/declare for a stateful
  // property affecting the _stateful_prop_id_to_prop_id vector among other
  // things.  This is necessary because a call to
  // getMaterialProperty[Old/Older] can potentially trigger a material to
  // become stateful that previously wasn't.  This needs to go after the
  // swapBack.
  initProps(tid, &elem, side, n_qpoints);

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
MaterialPropertyStorage::copy(const THREAD_ID tid,
                              const Elem & elem_to,
                              const Elem & elem_from,
                              unsigned int side,
                              unsigned int n_qpoints)
{
  copy(tid, &elem_to, &elem_from, side, n_qpoints);
}

void
MaterialPropertyStorage::copy(const THREAD_ID tid,
                              const Elem * elem_to,
                              const Elem * elem_from,
                              unsigned int side,
                              unsigned int n_qpoints)
{
  initProps(tid, elem_to, side, n_qpoints);

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
MaterialPropertyStorage::swap(const THREAD_ID tid, const Elem & elem, unsigned int side)
{
  Threads::spin_mutex::scoped_lock lock(this->_spin_mtx);

  for (const auto state : stateIndexRange())
    shallowSwapData(_stateful_prop_id_to_prop_id,
                    getMaterialData(tid).props(state),
                    // Would be nice to make this setProps()
                    initAndSetProps(&elem, side, state));
}

void
MaterialPropertyStorage::swapBack(const THREAD_ID tid, const Elem & elem, unsigned int side)
{
  Threads::spin_mutex::scoped_lock lock(this->_spin_mtx);

  for (const auto state : stateIndexRange())
    shallowSwapDataBack(_stateful_prop_id_to_prop_id,
                        setProps(&elem, side, state),
                        getMaterialData(tid).props(state));

  // Workaround for MOOSE difficulties in keeping materialless
  // elements (e.g. Lower D elements in Mortar code) materials
  for (const auto state : stateIndexRange())
    if (props(&elem, side, state).empty())
      setProps(state)[&elem].erase(side);
}

unsigned int
MaterialPropertyStorage::addProperty(const std::string & prop_name, const unsigned int state)
{
  if (state > MaterialData::max_state)
    mooseError("Material property state of ",
               state,
               " is not supported. Max state supported: ",
               MaterialData::max_state);

  // Increment state as needed
  if (maxState() < state)
    _max_state = state;

  const auto prop_id = _registry.addOrGetID(prop_name, {});

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

void
MaterialPropertyStorage::initProps(const THREAD_ID tid,
                                   const Elem * elem,
                                   unsigned int side,
                                   unsigned int n_qpoints)
{
  for (const auto state : stateIndexRange())
    this->initProps(tid, state, elem, side, n_qpoints);
}

void
MaterialPropertyStorage::initProps(const THREAD_ID tid,
                                   const unsigned int state,
                                   const Elem * elem,
                                   unsigned int side,
                                   unsigned int n_qpoints)
{
  auto & material_data = getMaterialData(tid);
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
      mat_props.setPointer(i, material_data.props(0)[prop_id].clone(n_qpoints), {});
      mooseAssert(mat_props[i].id() == prop_id, "Inconsistent id");
    }
}

void
dataStore(std::ostream & stream, MaterialPropertyStorage & storage, void * context)
{
  const auto num_states = storage.numStates();
  dataStore(stream, num_states, nullptr);

  // Store the material property ID -> name map for mapping back
  const auto & registry = storage.getMaterialPropertyRegistry();
  std::vector<std::string> ids_to_names(registry.idsToNamesBegin(), registry.idsToNamesEnd());
  dataStore(stream, ids_to_names, nullptr);

  // Store the stateful ID -> property ID map for mapping back
  dataStore(stream, storage._stateful_prop_id_to_prop_id, nullptr);

  // Store the stateful id -> name map
  dataStore(stream, storage._stateful_prop_names, nullptr);

  // Store every property
  for (const auto state : storage.stateIndexRange())
  {
    std::size_t num_elems = storage.setProps(state).size();
    dataStore(stream, num_elems, nullptr);

    for (auto & elem_side_map_pair : storage.setProps(state))
    {
      const Elem * elem = elem_side_map_pair.first;
      mooseAssert(elem, "Null element");
      dataStore(stream, elem, context);

      auto & side_map = elem_side_map_pair.second;
      std::size_t num_sides = side_map.size();
      dataStore(stream, num_sides, nullptr);

      for (auto & [side, props] : side_map)
      {
        dataStore(stream, side, nullptr);

        std::size_t num_props = props.size();
        dataStore(stream, num_props, nullptr);
        mooseAssert(num_props > 0, "No properties");

        std::size_t n_q_points = 0;
        for (const auto & entry : props)
          if (entry.size() > n_q_points)
            n_q_points = entry.size();
        dataStore(stream, n_q_points, nullptr);

        for (auto & entry : props)
          dataStoreSkippable(stream, entry, nullptr);
      }
    }
  }
}

void
dataLoad(std::istream & stream, MaterialPropertyStorage & storage, void * context)
{
  const auto & registry = storage.getMaterialPropertyRegistry();

  decltype(storage.numStates()) num_states;
  dataLoad(stream, num_states, nullptr);
  if (num_states != storage.numStates())
    mooseError("Stateful material properties up to state ",
               num_states,
               " were stored in checkpoint/backup,\nbut state ",
               storage.numStates(),
               " is now the maximum state requested.\n\nThis mismatch is not currently supported.");

  std::vector<std::string> from_prop_ids_to_names;
  dataLoad(stream, from_prop_ids_to_names, nullptr);

  decltype(storage._stateful_prop_id_to_prop_id) from_stateful_prop_id_to_prop_id;
  dataLoad(stream, from_stateful_prop_id_to_prop_id, nullptr);

  decltype(storage._stateful_prop_names) from_stateful_prop_names;
  dataLoad(stream, from_stateful_prop_names, nullptr);

  {
    const auto fill_names = [](const auto & from)
    {
      std::set<std::string> names;
      for (const auto & id_name_pair : from)
        names.insert(id_name_pair.second);
      return names;
    };

    const auto from_names = fill_names(from_stateful_prop_names);
    const auto to_names = fill_names(storage.statefulPropNames());

    // This requirement currently comes from the fact that now when we do restart/recover,
    // we do not call initStatefulQpProperties() anymore on each material. Which means that
    // if you add more stateful properties, they would never be initialized. It seems like
    // this capability (adding props) isn't currently used, so we're going to make it an
    // error for now. There are plenty of ways around this, but the priority at the moment
    // is enabling more flexible restart/recover, and this is one of the costs.
    // TODO: check types
    if (from_names != to_names)
    {
      std::stringstream err;
      err << "There is a mismatch in the stateful material properties stored during "
             "checkpoint/backup and declared in restart/recover.\n"
          << "The current system requires that the stateful properties be equivalent.\n\n";
      auto add_props = [&err](const auto & names)
      {
        for (const auto & name : names)
          err << "  " << name << "\n";
      };
      err << "Stored stateful properties:\n";
      add_props(from_names);
      err << "\nDeclared stateful properties:\n";
      add_props(to_names);
      mooseError(err.str());
    }
  }

  std::vector<std::optional<unsigned int>> to_stateful_ids(storage.statefulProps().size());

  // Fill the mapping from previous ID to current stateful ID
  for (const auto from_stateful_id : index_range(from_stateful_prop_id_to_prop_id))
  {
    const auto from_prop_id = from_stateful_prop_id_to_prop_id[from_stateful_id];

    mooseAssert(from_prop_id < from_prop_ids_to_names.size(), "Invalid ID map");
    const auto & from_name = from_prop_ids_to_names[from_prop_id];

    const auto to_prop_id = registry.getID(from_name);

    const auto find_prop_id = std::find(storage._stateful_prop_id_to_prop_id.begin(),
                                        storage._stateful_prop_id_to_prop_id.end(),
                                        to_prop_id);
    mooseAssert(find_prop_id != storage._stateful_prop_id_to_prop_id.end(), "Not found");

    const std::size_t to_stateful_id =
        std::distance(storage._stateful_prop_id_to_prop_id.begin(), find_prop_id);
    to_stateful_ids[from_stateful_id] = to_stateful_id;
  }

  // Load the properties
  for (const auto state : storage.stateIndexRange())
  {
    std::size_t num_elems;
    dataLoad(stream, num_elems, nullptr);

    for (std::size_t i_elem = 0; i_elem < num_elems; ++i_elem)
    {
      const Elem * elem;
      dataLoad(stream, elem, context);
      mooseAssert(elem, "Null element");

      std::size_t num_sides;
      dataLoad(stream, num_sides, nullptr);

      for (std::size_t i_side = 0; i_side < num_sides; ++i_side)
      {
        unsigned int side;
        dataLoad(stream, side, nullptr);

        std::size_t num_props;
        dataLoad(stream, num_props, nullptr);

        std::size_t num_q_points;
        dataLoad(stream, num_q_points, nullptr);

        storage.initProps(0, state, elem, side, num_q_points);
        auto & props = storage.setProps(elem, side, state);

        for (const auto from_stateful_id : make_range(num_props))
        {
          const auto to_stateful_id = to_stateful_ids[from_stateful_id];
          mooseAssert(to_stateful_id, "Skipping not supported");

          // Eventually we should support skipping, and this will be how it's done
          // if (!to_stateful_id)
          //   dataLoadSkip(stream);
          // else

          dataLoadSkippable(stream, props[*to_stateful_id], nullptr);
        }
      }
    }
  }
}
