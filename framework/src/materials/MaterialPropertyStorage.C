/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "MaterialPropertyStorage.h"
#include "Material.h"
#include "MaterialData.h"
#include "MooseMesh.h"

#include "libmesh/fe_interface.h"

std::map<std::string, unsigned int> MaterialPropertyStorage::_prop_ids;

/**
 * Shallow copy the material properties
 * @param stateful_prop_ids List of IDs with properties to shallow copy
 * @param data Destination data
 * @param data_from Source data
 */
void shallowCopyData(const std::vector<unsigned int> & stateful_prop_ids, MaterialProperties & data, MaterialProperties & data_from)
{
  for (unsigned int i=0; i<stateful_prop_ids.size(); ++i)
  {
    PropertyValue * prop = data[stateful_prop_ids[i]];              // do the look-up just once (OPT)
    PropertyValue * prop_from = data_from[i];                       // do the look-up just once (OPT)
    if (prop != NULL && prop_from != NULL)
      prop->swap(prop_from);
  }
}

void shallowCopyDataBack(const std::vector<unsigned int> & stateful_prop_ids, MaterialProperties & data, MaterialProperties & data_from)
{
  for (unsigned int i=0; i<stateful_prop_ids.size(); ++i)
  {
    PropertyValue * prop = data[i];                                 // do the look-up just once (OPT)
    PropertyValue * prop_from = data_from[stateful_prop_ids[i]];    // do the look-up just once (OPT)
    if (prop != NULL && prop_from != NULL)
      prop->swap(prop_from);
  }
}

MaterialPropertyStorage::MaterialPropertyStorage() :
    _has_stateful_props(false),
    _has_older_prop(false)
{
  _props_elem       = new HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> >;
  _props_elem_old   = new HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> >;
  _props_elem_older = new HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> >;
}

MaterialPropertyStorage::~MaterialPropertyStorage()
{
  releaseProperties();

  delete _props_elem;
  delete _props_elem_old;
  delete _props_elem_older;
}

void
MaterialPropertyStorage::releaseProperties()
{
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> >::iterator i;
  for (i = _props_elem->begin(); i != _props_elem->end(); ++i)
  {
    HashMap<unsigned int, MaterialProperties>::iterator j;
    for (j = i->second.begin(); j != i->second.end(); ++j)
      j->second.destroy();
  }

  for (i = _props_elem_old->begin(); i != _props_elem_old->end(); ++i)
  {
    HashMap<unsigned int, MaterialProperties>::iterator j;
    for (j = i->second.begin(); j != i->second.end(); ++j)
      j->second.destroy();
  }

  for (i = _props_elem_older->begin(); i != _props_elem_older->end(); ++i)
  {
    HashMap<unsigned int, MaterialProperties>::iterator j;
    for (j = i->second.begin(); j != i->second.end(); ++j)
      j->second.destroy();
  }
}

void
MaterialPropertyStorage::prolongStatefulProps(const std::vector<std::vector<QpMap> > & refinement_map, QBase & qrule, QBase & qrule_face, MaterialPropertyStorage & parent_material_props, MaterialData & child_material_data, const Elem & elem, const int input_parent_side, const int input_child, const int input_child_side)
{
  mooseAssert(input_child != -1 || input_parent_side == input_child_side, "Invalid inputs!");

  unsigned int n_qpoints = 0;

  // If we passed in -1 for these then we really need to store properties at 0
  unsigned int parent_side = input_parent_side == -1 ? 0 : input_parent_side;
  unsigned int child_side  = input_child_side  == -1 ? 0 : input_child_side;

  if(input_child_side == -1) // Not doing side projection (ie, doing volume projection)
    n_qpoints = qrule.n_points();
  else
    n_qpoints = qrule_face.n_points();

  child_material_data.size(n_qpoints);

  unsigned int n_children = elem.n_children();

  std::vector<unsigned int> children;

  if(input_child != -1) // Passed in a child explicitly
    children.push_back(input_child);
  else
  {
    children.resize(n_children);
    for(unsigned int child=0; child < n_children; child++)
      children[child] = child;
  }

  for(unsigned int i=0; i < children.size(); i++)
  {
    unsigned int child = children[i];

    // If we're not projecting an internal child side, but we are projecting sides, see if this child is on that side
    if(input_child == -1 && input_child_side != -1 && !elem.is_child_on_side(child, parent_side))
      continue;

    const Elem * child_elem = elem.child(child);

    const std::vector<QpMap> & child_map = refinement_map[child];

    if (props()[child_elem][child_side].size() == 0) props()[child_elem][child_side].resize(_stateful_prop_id_to_prop_id.size());
    if (propsOld()[child_elem][child_side].size() == 0) propsOld()[child_elem][child_side].resize(_stateful_prop_id_to_prop_id.size());
    if (propsOlder()[child_elem][child_side].size() == 0) propsOlder()[child_elem][child_side].resize(_stateful_prop_id_to_prop_id.size());

    // init properties (allocate memory. etc)
    for (unsigned int i=0; i < _stateful_prop_id_to_prop_id.size(); ++i)
    {
      // duplicate the stateful property in property storage (all three states - we will reuse the allocated memory there)
      // also allocating the right amount of memory, so we do not have to resize, etc.
      if (props()[child_elem][child_side][i] == NULL) props()[child_elem][child_side][i] = child_material_data.props()[ _stateful_prop_id_to_prop_id[i] ]->init(n_qpoints);
      if (propsOld()[child_elem][child_side][i] == NULL) propsOld()[child_elem][child_side][i] = child_material_data.propsOld()[ _stateful_prop_id_to_prop_id[i] ]->init(n_qpoints);
      if (hasOlderProperties())
        if (propsOlder()[child_elem][child_side][i] == NULL) propsOlder()[child_elem][child_side][i] = child_material_data.propsOlder()[ _stateful_prop_id_to_prop_id[i] ]->init(n_qpoints);

      // Copy from the parent stateful properties
      for(unsigned int qp=0; qp<refinement_map[child].size(); qp++)
      {
        PropertyValue * child_property = props()[child_elem][child_side][i];
        PropertyValue * parent_property = parent_material_props.props()[&elem][parent_side][i];

        child_property->qpCopy(qp, parent_property, child_map[qp]._to);
        propsOld()[child_elem][child_side][i]->qpCopy(qp, parent_material_props.propsOld()[&elem][parent_side][i], child_map[qp]._to);
        if (hasOlderProperties())
          propsOlder()[child_elem][child_side][i]->qpCopy(qp, parent_material_props.propsOlder()[&elem][parent_side][i], child_map[qp]._to);
      }
    }
  }
}



void
MaterialPropertyStorage::restrictStatefulProps(const std::vector<std::pair<unsigned int, QpMap> > & coarsening_map, std::vector<const Elem *> & coarsened_element_children, QBase & qrule, QBase & qrule_face, MaterialData & material_data, const Elem & elem, int input_side)
{
  unsigned int side;

  bool doing_a_side = input_side != -1;

  unsigned int n_qpoints = 0;

  if(!doing_a_side)
  {
    side = 0; // Use 0 for the elem
    n_qpoints = qrule.n_points();
  }
  else
  {
    side = input_side;
    n_qpoints = qrule_face.n_points();
  }

  material_data.size(n_qpoints);

  // First, make sure that storage has been set aside for this element.
  //initStatefulProps(material_data, mats, n_qpoints, elem, side);

  if (props()[&elem][side].size() == 0) props()[&elem][side].resize(_stateful_prop_id_to_prop_id.size());
  if (propsOld()[&elem][side].size() == 0) propsOld()[&elem][side].resize(_stateful_prop_id_to_prop_id.size());
  if (propsOlder()[&elem][side].size() == 0) propsOlder()[&elem][side].resize(_stateful_prop_id_to_prop_id.size());

  // init properties (allocate memory. etc)
  for (unsigned int i=0; i < _stateful_prop_id_to_prop_id.size(); ++i)
  {
    // duplicate the stateful property in property storage (all three states - we will reuse the allocated memory there)
    // also allocating the right amount of memory, so we do not have to resize, etc.
    if (props()[&elem][side][i] == NULL) props()[&elem][side][i] = material_data.props()[ _stateful_prop_id_to_prop_id[i] ]->init(n_qpoints);
    if (propsOld()[&elem][side][i] == NULL) propsOld()[&elem][side][i] = material_data.propsOld()[ _stateful_prop_id_to_prop_id[i] ]->init(n_qpoints);
    if (hasOlderProperties())
      if (propsOlder()[&elem][side][i] == NULL) propsOlder()[&elem][side][i] = material_data.propsOlder()[ _stateful_prop_id_to_prop_id[i] ]->init(n_qpoints);
  }

  // Copy from the child stateful properties
  for(unsigned int qp=0; qp<coarsening_map.size(); qp++)
  {
    const std::pair<unsigned int, QpMap> & qp_pair = coarsening_map[qp];
    unsigned int child = qp_pair.first;
    const Elem * child_elem = coarsened_element_children[child];
    const QpMap & qp_map = qp_pair.second;

    for (unsigned int i=0; i < _stateful_prop_id_to_prop_id.size(); ++i)
    {
      PropertyValue * child_property = props()[child_elem][side][i];
      PropertyValue * parent_property = props()[&elem][side][i];

      parent_property->qpCopy(qp, child_property, qp_map._to);

      propsOld()[&elem][side][i]->qpCopy(qp, propsOld()[child_elem][side][i], qp_map._to);
      if (hasOlderProperties())
        propsOlder()[&elem][side][i]->qpCopy(qp, propsOlder()[child_elem][side][i], qp_map._to);
    }
  }
}




void
MaterialPropertyStorage::initStatefulProps(MaterialData & material_data, std::vector<Material *> & mats, unsigned int n_qpoints, const Elem & elem, unsigned int side/* = 0*/)
{
  // NOTE: since materials are storing their computed properties in MaterialData class, we need to
  // juggle the memory between MaterialData and MaterialProperyStorage classes

  material_data.size(n_qpoints);

  if (props()[&elem][side].size() == 0) props()[&elem][side].resize(_stateful_prop_id_to_prop_id.size());
  if (propsOld()[&elem][side].size() == 0) propsOld()[&elem][side].resize(_stateful_prop_id_to_prop_id.size());
  if (propsOlder()[&elem][side].size() == 0) propsOlder()[&elem][side].resize(_stateful_prop_id_to_prop_id.size());

  // init properties (allocate memory. etc)
  for (unsigned int i=0; i < _stateful_prop_id_to_prop_id.size(); ++i)
  {
    // duplicate the stateful property in property storage (all three states - we will reuse the allocated memory there)
    // also allocating the right amount of memory, so we do not have to resize, etc.
    if (props()[&elem][side][i] == NULL) props()[&elem][side][i] = material_data.props()[ _stateful_prop_id_to_prop_id[i] ]->init(n_qpoints);
    if (propsOld()[&elem][side][i] == NULL) propsOld()[&elem][side][i] = material_data.propsOld()[ _stateful_prop_id_to_prop_id[i] ]->init(n_qpoints);
    if (hasOlderProperties())
      if (propsOlder()[&elem][side][i] == NULL) propsOlder()[&elem][side][i] = material_data.propsOlder()[ _stateful_prop_id_to_prop_id[i] ]->init(n_qpoints);
  }
  // copy from storage to material data
  swap(material_data, elem, side);
  // run custom init on properties
  for (std::vector<Material *>::iterator it = mats.begin(); it != mats.end(); ++it)
    (*it)->initStatefulProperties(n_qpoints);
  swapBack(material_data, elem, side);

  // Copy the properties to Old and Older as needed
  if (hasStatefulProperties())
    for (unsigned int i=0; i < _stateful_prop_id_to_prop_id.size(); ++i)
      for (unsigned int qp=0; qp < n_qpoints; ++qp)
      {
        propsOld()[&elem][side][i]->qpCopy(qp, props()[&elem][side][i], qp);
        if (hasOlderProperties())
          propsOlder()[&elem][side][i]->qpCopy(qp, props()[&elem][side][i], qp);
      }
}

void
MaterialPropertyStorage::shift()
{
  if (_has_older_prop)
  {
    // shift the properties back in time and reuse older for current (save reallocations etc.)
    HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > * tmp = _props_elem_older;
    _props_elem_older = _props_elem_old;
    _props_elem_old = _props_elem;
    _props_elem = tmp;
  }
  else
  {
    std::swap(_props_elem, _props_elem_old);
  }
}

void
MaterialPropertyStorage::swap(MaterialData & material_data, const Elem & elem, unsigned int side)
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

  shallowCopyData(_stateful_prop_id_to_prop_id, material_data.props(), props()[&elem][side]);
  shallowCopyData(_stateful_prop_id_to_prop_id, material_data.propsOld(), propsOld()[&elem][side]);
  if (hasOlderProperties())
    shallowCopyData(_stateful_prop_id_to_prop_id, material_data.propsOlder(), propsOlder()[&elem][side]);
}

void
MaterialPropertyStorage::swapBack(MaterialData & material_data, const Elem & elem, unsigned int side)
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

  shallowCopyDataBack(_stateful_prop_id_to_prop_id, props()[&elem][side], material_data.props());
  shallowCopyDataBack(_stateful_prop_id_to_prop_id, propsOld()[&elem][side], material_data.propsOld());
  if (hasOlderProperties())
    shallowCopyDataBack(_stateful_prop_id_to_prop_id, propsOlder()[&elem][side], material_data.propsOlder());
}

bool
MaterialPropertyStorage::hasProperty(const std::string & prop_name) const
{
  std::map<std::string, unsigned int>::const_iterator it = _prop_ids.find(prop_name);
  return (it != _prop_ids.end());
}

unsigned int
MaterialPropertyStorage::addProperty (const std::string & prop_name)
{
  unsigned int prop_id = addPropertyId(prop_name);
  _prop_names[prop_id] = prop_name;

  return prop_id;
}

unsigned int
MaterialPropertyStorage::addPropertyOld (const std::string & prop_name)
{
  unsigned int prop_id = addProperty(prop_name);
  _has_stateful_props = true;

  if (std::find(_stateful_prop_id_to_prop_id.begin(), _stateful_prop_id_to_prop_id.end(), prop_id) == _stateful_prop_id_to_prop_id.end())
    _stateful_prop_id_to_prop_id.push_back(prop_id);

  return prop_id;
}

unsigned int
MaterialPropertyStorage::addPropertyOlder (const std::string & prop_name)
{
  unsigned int prop_id = addProperty(prop_name);
  _has_stateful_props = true;
  _has_older_prop = true;

  if (std::find(_stateful_prop_id_to_prop_id.begin(), _stateful_prop_id_to_prop_id.end(), prop_id) == _stateful_prop_id_to_prop_id.end())
    _stateful_prop_id_to_prop_id.push_back(prop_id);

  return prop_id;
}

unsigned int
MaterialPropertyStorage::getPropertyId (const std::string & prop_name)
{
  std::map<std::string, unsigned int>::const_iterator it = _prop_ids.find(prop_name);
  if (it == _prop_ids.end())
    // The property hasn't been declared yet, so do it now.
    return addProperty(prop_name);
  else
    return it->second;
}

unsigned int
MaterialPropertyStorage::addPropertyId (const std::string & prop_name)
{
  std::map<std::string, unsigned int>::iterator it = _prop_ids.find(prop_name);
  if (it == _prop_ids.end())
  {
    unsigned int id = _prop_ids.size();
    _prop_ids[prop_name] = id;
    return id;
  }
  else
    return it->second;
}
