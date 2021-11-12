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

std::map<std::string, unsigned int> MaterialPropertyStorage::_prop_ids;

/**
 * Shallow copy the material properties
 * @param stateful_prop_ids List of IDs with properties to shallow copy
 * @param data Destination data
 * @param data_from Source data
 */
void
shallowCopyData(const std::vector<unsigned int> & stateful_prop_ids,
                MaterialProperties & data,
                MaterialProperties & data_from)
{
  for (unsigned int i = 0; i < stateful_prop_ids.size(); ++i)
  {
    if (i >= data_from.size() || stateful_prop_ids[i] >= data.size())
      continue;
    PropertyValue * prop = data[stateful_prop_ids[i]]; // do the look-up just once (OPT)
    PropertyValue * prop_from = data_from[i];          // do the look-up just once (OPT)
    if (prop != nullptr && prop_from != nullptr)
      prop->swap(prop_from);
  }
}

void
shallowCopyDataBack(const std::vector<unsigned int> & stateful_prop_ids,
                    MaterialProperties & data,
                    MaterialProperties & data_from)
{
  for (unsigned int i = 0; i < stateful_prop_ids.size(); ++i)
  {
    if (i >= data.size() || stateful_prop_ids[i] >= data_from.size())
      continue;
    PropertyValue * prop = data[i];                              // do the look-up just once (OPT)
    PropertyValue * prop_from = data_from[stateful_prop_ids[i]]; // do the look-up just once (OPT)
    if (prop != nullptr && prop_from != nullptr)
      prop->swap(prop_from);
  }
}

MaterialPropertyStorage::MaterialPropertyStorage()
  : _has_stateful_props(false), _has_older_prop(false)
{
  _props_elem =
      std::make_unique<HashMap<const Elem *, HashMap<unsigned int, MaterialProperties>>>();
  _props_elem_old =
      std::make_unique<HashMap<const Elem *, HashMap<unsigned int, MaterialProperties>>>();
  _props_elem_older =
      std::make_unique<HashMap<const Elem *, HashMap<unsigned int, MaterialProperties>>>();
}

MaterialPropertyStorage::~MaterialPropertyStorage() { releaseProperties(); }

void
MaterialPropertyStorage::releaseProperties()
{
  for (auto & i : *_props_elem)
    releasePropertyMap(i.second);

  for (auto & i : *_props_elem_old)
    releasePropertyMap(i.second);

  for (auto & i : *_props_elem_older)
    releasePropertyMap(i.second);
}

void
MaterialPropertyStorage::releasePropertyMap(HashMap<unsigned int, MaterialProperties> & inner_map)
{
  for (auto & i : inner_map)
    i.second.destroy();
}

void
MaterialPropertyStorage::eraseProperty(const Elem * elem)
{
  if (_props_elem->contains(elem))
    releasePropertyMap((*_props_elem)[elem]);
  _props_elem->erase(elem);

  if (_props_elem_old->contains(elem))
    releasePropertyMap((*_props_elem_old)[elem]);
  _props_elem_old->erase(elem);

  if (_props_elem_older->contains(elem))
    releasePropertyMap((*_props_elem_older)[elem]);
  _props_elem_older->erase(elem);
}

void
MaterialPropertyStorage::prolongStatefulProps(
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

    mooseAssert(child < refinement_map.size(), "Refinement_map vector not initialized");
    const std::vector<QpMap> & child_map = refinement_map[child];

    initProps(child_material_data, child_elem, child_side, n_qpoints);

    for (unsigned int i = 0; i < _stateful_prop_id_to_prop_id.size(); ++i)
    {
      // Copy from the parent stateful properties
      for (unsigned int qp = 0; qp < refinement_map[child].size(); qp++)
      {
        PropertyValue * child_property = props(child_elem, child_side)[i];
        mooseAssert(props().contains(&elem),
                    "Parent pointer is not in the MaterialProps data structure");
        PropertyValue * parent_property = parent_material_props.props(&elem, parent_side)[i];

        child_property->qpCopy(qp, parent_property, child_map[qp]._to);
        propsOld(child_elem, child_side)[i]->qpCopy(
            qp, parent_material_props.propsOld(&elem, parent_side)[i], child_map[qp]._to);
        if (hasOlderProperties())
          propsOlder(child_elem, child_side)[i]->qpCopy(
              qp, parent_material_props.propsOlder(&elem, parent_side)[i], child_map[qp]._to);
      }
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

  // Copy from the child stateful properties
  for (unsigned int qp = 0; qp < coarsening_map.size(); qp++)
  {
    const std::pair<unsigned int, QpMap> & qp_pair = coarsening_map[qp];
    unsigned int child = qp_pair.first;

    mooseAssert(child < coarsened_element_children.size(),
                "Coarsened element children vector not initialized");
    const Elem * child_elem = coarsened_element_children[child];
    const QpMap & qp_map = qp_pair.second;

    for (unsigned int i = 0; i < _stateful_prop_id_to_prop_id.size(); ++i)
    {
      mooseAssert(props().contains(child_elem),
                  "Child element pointer is not in the MaterialProps data structure");

      PropertyValue * child_property = props(child_elem, side)[i];
      PropertyValue * parent_property = props(&elem, side)[i];

      parent_property->qpCopy(qp, child_property, qp_map._to);

      propsOld(&elem, side)[i]->qpCopy(qp, propsOld(child_elem, side)[i], qp_map._to);
      if (hasOlderProperties())
        propsOlder(&elem, side)[i]->qpCopy(qp, propsOlder(child_elem, side)[i], qp_map._to);
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

  // Copy the properties to Old and Older as needed
  for (unsigned int i = 0; i < _stateful_prop_id_to_prop_id.size(); ++i)
  {
    auto curr = props(&elem, side)[i];
    auto old = propsOld(&elem, side)[i];
    auto older = propsOlder(&elem, side)[i];
    for (unsigned int qp = 0; qp < n_qpoints; ++qp)
    {
      old->qpCopy(qp, curr, qp);
      if (hasOlderProperties())
        older->qpCopy(qp, curr, qp);
    }
  }
}

void
MaterialPropertyStorage::shift()
{
  /**
   * Shift properties back in time and reuse older data for current (save reallocations etc.)
   * With current, old, and older this can be accomplished by two swaps:
   * older <-> old
   * old <-> current
   */
  if (_has_older_prop)
    std::swap(_props_elem_older, _props_elem_old);

  // Intentional fall through for case above and for handling just using old properties
  std::swap(_props_elem_old, _props_elem);
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
  for (unsigned int i = 0; i < _stateful_prop_id_to_prop_id.size(); ++i)
  {
    for (unsigned int qp = 0; qp < n_qpoints; ++qp)
    {
      props(elem_to, side)[i]->qpCopy(qp, props(elem_from, side)[i], qp);
      propsOld(elem_to, side)[i]->qpCopy(qp, propsOld(elem_from, side)[i], qp);
      if (hasOlderProperties())
        propsOlder(elem_to, side)[i]->qpCopy(qp, propsOlder(elem_from, side)[i], qp);
    }
  }
}

void
MaterialPropertyStorage::swap(MaterialData & material_data, const Elem & elem, unsigned int side)
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

  shallowCopyData(_stateful_prop_id_to_prop_id, material_data.props(), props(&elem, side));
  shallowCopyData(_stateful_prop_id_to_prop_id, material_data.propsOld(), propsOld(&elem, side));
  if (hasOlderProperties())
    shallowCopyData(
        _stateful_prop_id_to_prop_id, material_data.propsOlder(), propsOlder(&elem, side));
}

void
MaterialPropertyStorage::swapBack(MaterialData & material_data,
                                  const Elem & elem,
                                  unsigned int side)
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

  shallowCopyDataBack(_stateful_prop_id_to_prop_id, props(&elem, side), material_data.props());
  shallowCopyDataBack(
      _stateful_prop_id_to_prop_id, propsOld(&elem, side), material_data.propsOld());
  if (hasOlderProperties())
    shallowCopyDataBack(
        _stateful_prop_id_to_prop_id, propsOlder(&elem, side), material_data.propsOlder());
}

bool
MaterialPropertyStorage::hasProperty(const std::string & prop_name) const
{
  return _prop_ids.count(prop_name) > 0;
}

unsigned int
MaterialPropertyStorage::addProperty(const std::string & prop_name)
{
  return getPropertyId(prop_name);
}

unsigned int
MaterialPropertyStorage::addPropertyOld(const std::string & prop_name)
{
  unsigned int prop_id = addProperty(prop_name);
  _has_stateful_props = true;

  if (std::find(_stateful_prop_id_to_prop_id.begin(),
                _stateful_prop_id_to_prop_id.end(),
                prop_id) == _stateful_prop_id_to_prop_id.end())
    _stateful_prop_id_to_prop_id.push_back(prop_id);
  _prop_names[prop_id] = prop_name;

  return prop_id;
}

unsigned int
MaterialPropertyStorage::addPropertyOlder(const std::string & prop_name)
{
  _has_older_prop = true;
  return addPropertyOld(prop_name);
}

unsigned int
MaterialPropertyStorage::getPropertyId(const std::string & prop_name)
{
  auto it = _prop_ids.find(prop_name);
  if (it != _prop_ids.end())
    return it->second;

  auto id = _prop_ids.size();
  _prop_ids[prop_name] = id;
  return id;
}

unsigned int
MaterialPropertyStorage::retrievePropertyId(const std::string & prop_name) const
{
  auto it = _prop_ids.find(prop_name);
  if (it == _prop_ids.end())
    mooseError("MaterialPropertyStorage: property " + prop_name + " is not yet declared");
  return it->second;
}

void
MaterialPropertyStorage::initProps(MaterialData & material_data,
                                   const Elem * elem,
                                   unsigned int side,
                                   unsigned int n_qpoints)
{
  material_data.resize(n_qpoints);
  auto n = _stateful_prop_id_to_prop_id.size();

  // In some special cases, material_data might be larger than n_qpoints
  if (material_data.isOnlyResizeIfSmaller())
    n_qpoints = material_data.nQPoints();

  if (props(elem, side).size() < n)
    props(elem, side).resize(n, nullptr);
  if (propsOld(elem, side).size() < n)
    propsOld(elem, side).resize(n, nullptr);
  if (propsOlder(elem, side).size() < n)
    propsOlder(elem, side).resize(n, nullptr);

  // init properties (allocate memory. etc)
  for (unsigned int i = 0; i < n; i++)
  {
    auto prop_id = _stateful_prop_id_to_prop_id[i];
    // duplicate the stateful property in property storage (all three states - we will reuse the
    // allocated memory there)
    // also allocating the right amount of memory, so we do not have to resize, etc.
    if (props(elem, side)[i] == nullptr)
      props(elem, side)[i] = material_data.props()[prop_id]->init(n_qpoints);
    if (propsOld(elem, side)[i] == nullptr)
      propsOld(elem, side)[i] = material_data.propsOld()[prop_id]->init(n_qpoints);
    if (hasOlderProperties() && propsOlder(elem, side)[i] == nullptr)
      propsOlder(elem, side)[i] = material_data.propsOlder()[prop_id]->init(n_qpoints);
  }
}
