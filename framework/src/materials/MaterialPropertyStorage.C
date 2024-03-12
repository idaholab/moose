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
  : _max_state(0),
    _spin_mtx(libMesh::Threads::spin_mtx),
    _registry(registry),
    _restart_in_place(false),
    _recovering(false)
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

void
MaterialPropertyStorage::eraseProperty(const Elem * elem)
{
  for (const auto state : stateIndexRange())
    setProps(state).erase(elem);
}

void
MaterialPropertyStorage::setRestartInPlace()
{
  _restart_in_place = true;
  _restartable_map.clear();
}

const MaterialPropertyStorage::PropRecord &
MaterialPropertyStorage::getPropRecord(const unsigned int id) const
{
  mooseAssert(_prop_records.size() > id, "Invalid ID");
  auto & prop_record_ptr = _prop_records[id];
  mooseAssert(prop_record_ptr, "Not initialized");
  return *prop_record_ptr;
}

bool
MaterialPropertyStorage::isRestoredProperty(const std::string & name) const
{
  const auto & record = getPropRecord(_registry.getID(name));
  if (!record.stateful())
    mooseAssert(!record.restored, "Stateful properties should not be restored");
  return record.restored;
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

  auto current_p_level_props = initProps(tid, &elem, side, n_qpoints);

  for (const auto state : stateIndexRange())
    for (const auto i : index_range(_stateful_prop_id_to_prop_id))
    {
      auto & current_p_level_prop = (*current_p_level_props[state])[i];
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

    auto child_props = initProps(tid, child_elem, child_side, n_qpoints);

    for (const auto state : stateIndexRange())
    {
      const auto & parent_props = parent_material_props.props(&elem, parent_side, state);
      for (const auto i : index_range(_stateful_prop_id_to_prop_id))
        for (const auto qp : index_range(refinement_map[child]))
          (*child_props[state])[i].qpCopy(qp, parent_props[i], child_map[qp]._to);
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

  auto parent_props = initProps(tid, &elem, side, n_qpoints);

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
  // Material -> stateful properties that need to be restarted for said material
  // We need this because we need to initialize everything in dependency the dependency ordering
  // as given in mats, which means that we can't just do these all up front
  std::unordered_map<const MaterialBase *,
                     std::vector<std::pair<unsigned int, std::vector<std::stringstream> *>>>
      materials_to_restart;
  // Find the entry in the restartable data (binary data waiting to be restarted)
  // for this [elem, side], if any
  RestartableMapType::mapped_type * restartable_entry = nullptr;
  if (_restartable_map.size())
    if (auto it = _restartable_map.find(std::make_pair(&elem, side)); it != _restartable_map.end())
      restartable_entry = &it->second;

  // The stateful objects that we're going to initialize. This is needed for materials that are
  // passed in as mat that are not stateful, who we don't want to call initStatefulProperties() on.
  // We unfortunately can only determine this here due to block/boundary restriction, which is
  // not known without mapping property -> material
  std::vector<MaterialBase *> stateful_mats;
  // The stateful IDs that we need to copy back
  std::vector<unsigned int> stateful_ids_to_copy;

#ifndef NDEBUG
  // All of the stateful IDs; used to make sure that we're not supplying one property
  // across multiple materials
  std::set<unsigned int> stateful_ids;
#endif

  // Work through all of the materials that we were passed to figure out which ones are stateful
  // that we need to initialize, and also to figure out which ones we need to restart
  for (const auto & mat : mats)
  {
    // For keeping track of this material in stateful_mats
    bool stateful = false;

    // Check each material that was declared
    for (const auto id : mat->getSuppliedPropIDs())
    {
      const auto & record = getPropRecord(id);
      if (record.stateful())
      {
        const auto stateful_id = record.stateful_id;
        stateful = true;

#ifndef NDEBUG
        const auto it_inserted_pair = stateful_ids.insert(stateful_id);
        mooseAssert(it_inserted_pair.second,
                    "Material property '" + _registry.getName(id) +
                        "' supplied by multiple materials at the same point");
#endif

        bool restarting = false;
        // We have restartable data for this [elem, side]; see if we have it for this prop
        if (restartable_entry)
        {
          if (auto id_datum_pair_it = restartable_entry->find(stateful_id);
              id_datum_pair_it != restartable_entry->end())
          {
            restarting = true;
            materials_to_restart[mat.get()].emplace_back(stateful_id, &id_datum_pair_it->second);
          }
        }

        if (!restarting)
          stateful_ids_to_copy.push_back(record.stateful_id);
      }
    }

    if (stateful || mat->forceStatefulInit())
      stateful_mats.push_back(mat.get());
  }

  // This currently initializes all of the stateful properties for this [elem, side],
  // even though we didn't necessarily need to init all of them. Future work?
  auto props = initProps(tid, &elem, side, n_qpoints);

  // Whether or not we have swapped material data from _storage to _material_data
  // When we're initializing from initStatefulProperties, the MaterialBase objects
  // need to be able to access the data in _material_data. When we're initalizing
  // from restart, we need to be able to access from _storage.
  bool swapped = false;

  // Initialize properties
  for (auto mat : stateful_mats)
  {
    const auto materials_to_restart_find = materials_to_restart.find(mat);
    const bool restart = materials_to_restart_find != materials_to_restart.end();

    if (!restart || mat->forceStatefulInit())
    {
      // Need stateful _material_data in the material to store
      if (!swapped)
      {
        swap(tid, elem, side);
        swapped = true;
      }

      mat->initStatefulProperties(n_qpoints);
    }

    if (restart)
    {
      // Need data in _storage
      if (swapped)
      {
        swapBack(tid, elem, side);
        swapped = false;
      }

      // Load from the cached binary backup into place
      for (auto & [stateful_id, datum_ptr] : materials_to_restart_find->second)
        for (const auto state : index_range(*datum_ptr))
        {
          (*datum_ptr)[state].seekg(0, std::ios::beg);
          dataLoad((*datum_ptr)[state], (*props[state])[stateful_id], nullptr);
        }
    }
  }

  // Done with accessing from _material_data
  if (swapped)
    swapBack(tid, elem, side);

  // Copy to older states as needed, only for non-restarted data
  for (const auto stateful_id : stateful_ids_to_copy)
    for (const auto state : statefulIndexRange())
      for (const auto qp : make_range(n_qpoints))
        (*props[state])[stateful_id].qpCopy(qp, (*props[0])[stateful_id], qp);
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
  auto to_props = initProps(tid, elem_to, side, n_qpoints);

  for (const auto state : stateIndexRange())
  {
    const auto & from_props = props(elem_from, side, state);
    for (const auto i : index_range(_stateful_prop_id_to_prop_id))
      for (const auto qp : make_range(n_qpoints))
        (*to_props[state])[i].qpCopy(qp, from_props[i], qp);
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
  record.type = type.name();
  if (declarer)
    record.declarers.emplace(declarer->typeAndName());
  record.state = std::max(state, record.state);

  // Keep track of stateful props by quick access
  if (state > 0 && std::find(_stateful_prop_id_to_prop_id.begin(),
                             _stateful_prop_id_to_prop_id.end(),
                             prop_id) == _stateful_prop_id_to_prop_id.end())
  {
    _stateful_prop_id_to_prop_id.push_back(prop_id);
    record.stateful_id = _stateful_prop_id_to_prop_id.size() - 1;
  }

  return prop_id;
}

std::vector<MaterialProperties *>
MaterialPropertyStorage::initProps(const THREAD_ID tid,
                                   const Elem * elem,
                                   unsigned int side,
                                   unsigned int n_qpoints)
{
  std::vector<MaterialProperties *> props(numStates());
  for (const auto state : stateIndexRange())
    props[state] = &this->initProps(tid, state, elem, side, n_qpoints);
  return props;
}

MaterialProperties &
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

  return mat_props;
}

void
dataStore(std::ostream & stream, MaterialPropertyStorage & storage, void * context)
{
  // Store the material property ID -> name map for mapping back
  const auto & registry = storage.getMaterialPropertyRegistry();
  std::vector<std::string> ids_to_names(registry.idsToNamesBegin(), registry.idsToNamesEnd());
  dataStore(stream, ids_to_names, nullptr);

  // Store the stateful ID -> property ID map for mapping back
  dataStore(stream, storage._stateful_prop_id_to_prop_id, nullptr);

  // Store the prop -> record map
  dataStore(stream, storage._prop_records, nullptr);

  // Store the number of states
  auto num_states = storage.numStates();
  dataStore(stream, num_states, nullptr);

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

        // Here we actually store a stringstream of the data instead of the data directly, because
        // upon load we don't know if we can load it immediately into place or if we need to cache
        // it to load later. We also store it as skippable so that we can support not loading a
        // property if it no longer exists in restart
        for (auto & entry : props)
        {
          std::stringstream out;
          dataStore(out, entry, nullptr);
          dataStore(stream, out, nullptr);
        }
      }
    }
  }
}

void
dataLoad(std::istream & stream, MaterialPropertyStorage & storage, void * context)
{
  storage._restartable_map.clear();

  const auto & registry = storage.getMaterialPropertyRegistry();

  std::vector<std::string> from_prop_ids_to_names;
  dataLoad(stream, from_prop_ids_to_names, nullptr);

  decltype(storage._stateful_prop_id_to_prop_id) from_stateful_prop_id_to_prop_id;
  dataLoad(stream, from_stateful_prop_id_to_prop_id, nullptr);

  decltype(storage._prop_records) from_prop_records;
  dataLoad(stream, from_prop_records, nullptr);

  decltype(storage.numStates()) num_states;
  dataLoad(stream, num_states, nullptr);

  {
    // Build maps of material object -> properties and property -> material objects
    const auto build_maps = [](const auto & prop_records, const auto & ids_to_names)
    {
      std::map<std::string, std::set<std::string>> object_to_props, prop_to_objects;
      for (const auto i : index_range(prop_records))
        if (prop_records[i] && prop_records[i]->stateful())
        {
          const auto & prop = ids_to_names[i];
          for (const auto & declarer : (*prop_records[i]).declarers)
          {
            object_to_props[declarer].insert(prop);
            prop_to_objects[prop].insert(declarer);
          }
        }

      return std::make_pair(std::move(object_to_props), std::move(prop_to_objects));
    };
    // Maps for the current stateful properties
    const std::vector<std::string> prop_ids_to_names(registry.idsToNamesBegin(),
                                                     registry.idsToNamesEnd());
    const auto [object_to_props, prop_to_objects] =
        build_maps(storage._prop_records, prop_ids_to_names);
    // Maps for the stored stateful properties
    const auto [from_object_to_props, from_prop_to_objects] =
        build_maps(from_prop_records, from_prop_ids_to_names);

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
          error << "The stateful material properties in " << object
                << " that are being restarted do not match the stored properties in the same "
                   "material object from the checkpoint.\n\n";
          error << "Checkpointed stateful properties:\n";
          for (const auto & prop : from_props)
            error << " - " << prop << "\n";
          error << "\nCurrent stateful properties:\n";
          for (const auto & prop : props)
            error << " - " << prop << "\n";
          mooseError(error.str());
        }
      }
      // We're recovering and we haven't found a stateful material object. We require a
      // one-to-one stateful mapping when recovering
      else if (storage._recovering)
      {
        mooseError("The ",
                   object,
                   " was stored in restart but no longer exists. This is not supported when "
                   "recovering stateful material properties.");
      }
    }

    // We can easily support this, but have chosen not to due to ambiguity and we
    // don't yet know how to express this ambiguity. Just removing this error
    // _should_ make it work without issue because to_stateful_ids below for this
    // property will be an empty optional, which means simply don't load it. Or,
    // we could load it into the new property.
    for (const auto & [from_prop, from_objects] : from_prop_to_objects)
      if (const auto find_objects = prop_to_objects.find(from_prop);
          find_objects != prop_to_objects.end())
        for (const auto & object : find_objects->second)
          if (!from_objects.count(object))
            mooseError(
                "The stateful material property '",
                from_prop,
                "' was declared in ",
                object,
                " but was not declared in that object on checkpoint.\n\nThis is not currently "
                "supported due to ambiguity.\n\nPlease contact the development team on "
                "GitHub if you desire this capability.");
  }

  std::vector<std::optional<unsigned int>> to_stateful_ids(from_stateful_prop_id_to_prop_id.size());

  auto & to_prop_records = storage._prop_records;

  // Mark everything as not restored in the event that we call this again
  for (auto & record_ptr : to_prop_records)
    if (record_ptr)
      record_ptr->restored = false;

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
          mooseError(
              "The type for the restarted stateful material property '", name, "' does not match");

        // I'm not sure if we need to enforce this one, but I don't want to think
        // about it deeply so we'll just make it an error until someone complains
        // we have time to think
        if (from_record.state != to_record.state)
          mooseError("The number of states for the restarted stateful material property '",
                     name,
                     "' do not match.\n\n",
                     "Checkpointed states: ",
                     from_record.state,
                     "\nCurrent states: ",
                     to_record.state);

        const auto to_stateful_id = storage.getPropRecord(to_prop_id).stateful_id;
        mooseAssert(to_stateful_id != invalid_uint, "Not stateful");

        to_stateful_ids[from_stateful_id] = to_stateful_id;

        // Mark that we're going to restore this property
        to_record.restored = true;
        mooseAssert(storage.isRestoredProperty(name), "Restored mismatch");

        if (storage._recovering)
          mooseAssert(from_stateful_id == to_stateful_id, "Does not have direct mapping");
      }
    }
  }

  // Load the properties
  for (const auto state : make_range(num_states))
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
        mooseAssert(num_props <= to_stateful_ids.size(), "Missized map");

        std::size_t num_q_points;
        dataLoad(stream, num_q_points, nullptr);

        // Avoid multiple map lookups for entries pertaining to [elem, side]
        MaterialPropertyStorage::RestartableMapType::mapped_type * restart_entry = nullptr;
        MaterialProperties * in_place_entry = nullptr;

        // Load each stateful property
        for (const auto from_stateful_id : make_range(num_props))
        {
          // Load the binary data, which lets us choose in a moment whether
          // or not to load it in place or to cache it to load later
          std::stringstream data;
          dataLoad(stream, data, nullptr);
          data.seekg(0, std::ios::beg);

          // We have a property to load into
          if (const auto to_stateful_id_ptr = to_stateful_ids[from_stateful_id])
          {
            const auto to_stateful_id = *to_stateful_id_ptr;

            // Load the data directly into _storage
            if (storage._restart_in_place)
            {
              if (!in_place_entry)
                in_place_entry = &storage.setProps(elem, side, state);

              dataLoad(data, (*in_place_entry)[to_stateful_id], nullptr);
            }
            // Properties aren't initialized, so load the data into
            // _restartable_map to be loaded later in initStatefulProps()
            else
            {
              if (!restart_entry)
                restart_entry = &storage._restartable_map[std::make_pair(elem, side)];

              auto & mat_entry = (*restart_entry)[to_stateful_id];
              if (state >= mat_entry.size())
                mat_entry.resize(state + 1);

              mat_entry[state] = std::move(data);
            }
          }
        }
      }
    }
  }
}

void
dataStore(std::ostream & stream, MaterialPropertyStorage::PropRecord & record, void *)
{
  dataStore(stream, record.declarers, nullptr);
  dataStore(stream, record.type, nullptr);
  dataStore(stream, record.stateful_id, nullptr);
  dataStore(stream, record.state, nullptr);
}

void
dataLoad(std::istream & stream, MaterialPropertyStorage::PropRecord & record, void *)
{
  dataLoad(stream, record.declarers, nullptr);
  dataLoad(stream, record.type, nullptr);
  dataLoad(stream, record.stateful_id, nullptr);
  dataLoad(stream, record.state, nullptr);
}
