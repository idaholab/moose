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
#include "MaterialBase.h"
#include "DataIO.h"

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

std::optional<std::string>
MaterialPropertyStorage::queryStatefulPropName(const unsigned int id) const
{
  if (_prop_records.size() > id && _prop_records[id] && _prop_records[id]->stateful())
    return _registry.getName(id);
  return {};
}

bool
MaterialPropertyStorage::isStatefulProp(const std::string & prop_name) const
{
  const auto id = _registry.getID(prop_name);
  return _prop_records.size() > id && _prop_records[id] && _prop_records[id]->stateful();
}

void
MaterialPropertyStorage::eraseProperty(const Elem * elem)
{
  for (const auto state : stateIndexRange())
    setProps(state).erase(elem);
}

void
MaterialPropertyStorage::updateStatefulPropsForPRefinement(
    const processor_id_type libmesh_dbg_var(pid),
    const std::vector<QpMap> & p_refinement_map,
    const QBase & qrule,
    const QBase & qrule_face,
    const THREAD_ID tid,
    const Elem & elem,
    const int input_side)
{
  unsigned int n_qpoints = 0;

  // If we passed in -1 for these then we really need to store properties at 0
  unsigned int side = input_side == -1 ? 0 : input_side;

  if (input_side == -1) // Not doing side projection (ie, doing volume projection)
    n_qpoints = qrule.n_points();
  else
    n_qpoints = qrule_face.n_points();

  getMaterialData(tid).resize(n_qpoints);

  mooseAssert(elem.active(), "We should be doing p-refinement on active elements only");
  mooseAssert(elem.processor_id() == pid, "Prolongation should be occurring locally");
  mooseAssert(p_refinement_map.size() == n_qpoints, "Refinement map not proper size");

  initProps(tid, &elem, side, n_qpoints);

  for (const auto state : stateIndexRange())
  {
    auto & props = setProps(&elem, side, state);
    for (const auto i : index_range(_stateful_prop_id_to_prop_id))
    {
      auto & current_p_level_prop = props[i];
      // We need to clone this property in order to not overwrite the values we're going to be
      // reading from
      auto previous_p_level_prop = current_p_level_prop.clone(current_p_level_prop.size());
      // Cloning, despite its name, does not copy the data. Luckily since we are about to overwrite
      // all of the current_p_level_prop data, we can just swap its data over to our
      // previous_p_level_prop
      previous_p_level_prop->swap(current_p_level_prop);
      current_p_level_prop.resize(n_qpoints);
      for (const auto qp : index_range(p_refinement_map))
        current_p_level_prop.qpCopy(qp, *previous_p_level_prop, p_refinement_map[qp]._to);
    }
  }
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
  // we don't call this for properties that we've already restored because that
  // would overwrite what we've loaded
  for (const auto & mat : mats)
    if (!_restored_materials.count(mat.get()))
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
MaterialPropertyStorage::addProperty(const std::string & prop_name,
                                     const std::type_info & type,
                                     const unsigned int state,
                                     const MaterialBase * const declarer)
{
  if (state > MaterialData::max_state)
    mooseError("Material property state of ",
               state,
               " is not supported. Max state supported: ",
               MaterialData::max_state);

  // Increment state as needed
  if (maxState() < state)
    _max_state = state;

  // Register the property
  const auto prop_id = _registry.addOrGetID(prop_name, {});

  // Instantiate the record if needed
  if (_prop_records.size() < _registry.size())
    _prop_records.resize(_registry.size());
  if (!_prop_records[prop_id])
    _prop_records[prop_id] = PropRecord();

  // Fill the record
  auto & record = *_prop_records[prop_id];
  if (declarer)
    record.declarer = declarer;
  if (state > record.state)
    record.state = state;
  record.type = type.name();

  // Keep track of stateful props by quick access
  if (state > 0 && std::find(_stateful_prop_id_to_prop_id.begin(),
                             _stateful_prop_id_to_prop_id.end(),
                             prop_id) == _stateful_prop_id_to_prop_id.end())
    _stateful_prop_id_to_prop_id.push_back(prop_id);

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

  // Store the prop -> record map
  dataStore(stream, storage._prop_records, nullptr);

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
  storage._restored_materials.clear();

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

  decltype(storage._prop_records) from_prop_records;
  dataLoad(stream, from_prop_records, nullptr);

  // Common type for representing a Material; type and name
  using MaterialObject = std::pair<std::string, std::string>;

  {
    // Build maps of material object -> properties and property -> material object
    const auto build_maps = [](const auto & prop_records, const auto & ids_to_names)
    {
      std::map<MaterialObject, std::set<std::string>> object_to_props;
      std::map<std::string, MaterialObject> prop_to_object;
      for (const auto i : index_range(prop_records))
        if (prop_records[i] && prop_records[i]->stateful())
        {
          const auto & record = *prop_records[i];
          MaterialObject object;
          if (const auto material_ptr = std::get_if<const MaterialBase *>(&record.declarer))
            object = std::make_pair((*material_ptr)->type(), (*material_ptr)->name());
          else if (const auto material_name = std::get_if<MaterialObject>(&record.declarer))
            object = *material_name;
          const auto & name = ids_to_names[i];
          object_to_props[object].insert(name);
          prop_to_object.emplace(name, object);
        }

      return std::make_pair(object_to_props, prop_to_object);
    };
    // Maps for the current stateful properties
    const std::vector<std::string> prop_ids_to_names(registry.idsToNamesBegin(),
                                                     registry.idsToNamesEnd());
    const auto [object_to_props, prop_to_object] =
        build_maps(storage._prop_records, prop_ids_to_names);
    // Maps for the stored stateful properties
    const auto [from_object_to_props, from_prop_to_object] =
        build_maps(from_prop_records, from_prop_ids_to_names);

    // Helper for printing object names in errors
    const auto object_string = [](const MaterialObject & object)
    { return object.first + " '" + object.second + "'"; };

    // Enforce our stateful requirements
    for (const auto & [object, props] : object_to_props)
    {
      const auto find_from_object = from_object_to_props.find(object);

      // We have a material object that was stored with the same name that
      // had stateful material properties. Here, we enforce that the stateful
      // properties stored match exactly the ones that we have declared in
      // the new run
      if (find_from_object != from_object_to_props.end())
      {
        const auto & from_props = find_from_object->second;
        if (props != from_props)
        {
          std::stringstream error;
          error << "The stateful material properties in " << object_string(object)
                << " that are being restarted do not match the stored properties in the same "
                   "material object from the checkpoint.\n\n";
          error << "Checkpointed stateful properties:\n";
          for (const auto & prop : from_props)
            error << " - " << prop << "\n";
          error << "\nCurrent stateful properties:\n";
          for (const auto & prop : props)
            error << " - " << prop << "\n";
          error << "\nThis is not supported in advanced restart.";
          mooseError(error.str());
        }
      }
      // We have not found a material object that was stored with the same name
      // with stateful material properties. Here, we enforce that no other new
      // stateful material properties are declared in a new material with the
      // same name to avoid ambiguity.
      else
      {
        for (const auto & prop : props)
          if (const auto find = from_prop_to_object.find(prop);
              from_prop_to_object.find(prop) != from_prop_to_object.end())
            mooseError("The stateful material property '",
                       prop,
                       "' was stored in checkpoint in ",
                       object_string(find->second),
                       " but is now declared in ",
                       object_string(object),
                       ".\n\nThis is not supported in advanced restart due to ambiguity.\n\n");
      }
    }
  }

  std::vector<std::optional<unsigned int>> to_stateful_ids(storage.statefulProps().size());

  auto & to_prop_records = storage._prop_records;

  // Fill the mapping from previous ID to current stateful ID
  for (const auto from_stateful_id : index_range(from_stateful_prop_id_to_prop_id))
  {
    const auto from_prop_id = from_stateful_prop_id_to_prop_id[from_stateful_id];

    mooseAssert(from_prop_id < from_prop_records.size(), "Invalid record map");
    mooseAssert(from_prop_records[from_prop_id], "Not set");
    const auto & from_record = *from_prop_records[from_prop_id];
    mooseAssert(from_record.stateful(), "Not stateful");

    mooseAssert(from_prop_id < from_prop_ids_to_names.size(), "Invalid ID map");
    const auto & name = from_prop_ids_to_names[from_prop_id];

    if (const auto query_to_prop_id = registry.queryID(name))
    {
      const auto to_prop_id = *query_to_prop_id;

      mooseAssert(to_prop_id < to_prop_records.size(), "Invalid record map");
      mooseAssert(to_prop_records[to_prop_id], "Not set");
      auto & to_record = *to_prop_records[to_prop_id];

      if (to_record.stateful())
      {
        if (from_record.type != to_record.type)
          mooseError("The type for the restarted stateful material property '",
                     name,
                     "' in does not match");
        if (from_record.state != to_record.state)
          mooseError("The number of states for the restarted stateful material property '",
                     name,
                     "' do not match.\n\n",
                     "Checkpointed states: ",
                     from_record.state,
                     "\nCurrent states: ",
                     to_record.state);

        const auto find_prop_id = std::find(storage._stateful_prop_id_to_prop_id.begin(),
                                            storage._stateful_prop_id_to_prop_id.end(),
                                            to_prop_id);
        mooseAssert(find_prop_id != storage._stateful_prop_id_to_prop_id.end(), "Not found");

        const std::size_t to_stateful_id =
            std::distance(storage._stateful_prop_id_to_prop_id.begin(), find_prop_id);
        to_stateful_ids[from_stateful_id] = to_stateful_id;

        const auto material_ptr = std::get_if<const MaterialBase *>(&to_record.declarer);
        mooseAssert(material_ptr && *material_ptr, "Should have a declarer");
        storage._restored_materials.insert(*material_ptr);
      }
    }
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
          if (const auto to_stateful_id_ptr = to_stateful_ids[from_stateful_id])
            dataLoadSkippable(stream, props[*to_stateful_id_ptr], nullptr);
          else
            dataLoadSkip(stream);
      }
    }
  }
}

void
dataStore(std::ostream & stream, MaterialPropertyStorage::PropRecord & record, void *)
{
  const auto material_ptr = std::get_if<const MaterialBase *>(&record.declarer);
  mooseAssert(material_ptr, "Should have a material");

  std::string declarer_type = (*material_ptr)->type();
  dataStore(stream, declarer_type, nullptr);
  std::string declarer_name = (*material_ptr)->name();
  dataStore(stream, declarer_name, nullptr);

  dataStore(stream, record.type, nullptr);
  dataStore(stream, record.state, nullptr);
}

void
dataLoad(std::istream & stream, MaterialPropertyStorage::PropRecord & record, void *)
{
  std::pair<std::string, std::string> declarer;
  dataLoad(stream, declarer.first, nullptr);
  dataLoad(stream, declarer.second, nullptr);
  record.declarer = declarer;

  dataLoad(stream, record.type, nullptr);
  dataLoad(stream, record.state, nullptr);
}
