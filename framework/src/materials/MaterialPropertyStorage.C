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

/**
 * Shallow copy the material properties
 * @param prop_ids List of IDs with properties to shallow copy
 * @param data Destination data
 * @param data_from Source data
 */
void shallowCopyData(const std::set<unsigned int> & prop_ids,
                     MaterialProperties & data,
                     MaterialProperties & data_from)
{
  for (std::set<unsigned int>::const_iterator it = prop_ids.begin(); it != prop_ids.end(); ++it)
  {
    unsigned int id = *it;
    PropertyValue * prop = data[id];                  // do the look-up just once (OPT)
    PropertyValue * prop_from = data_from[id];        // do the look-up just once (OPT)
    if (prop != NULL && prop_from != NULL)
      prop->shallowCopy(prop_from);
  }
}


MaterialPropertyStorage::MaterialPropertyStorage() :
    _has_stateful_props(false),
    _has_older_prop(false)
{
  _props_elem       = new HashMap<unsigned int, HashMap<unsigned int, MaterialProperties> >;
  _props_elem_old   = new HashMap<unsigned int, HashMap<unsigned int, MaterialProperties> >;
  _props_elem_older = new HashMap<unsigned int, HashMap<unsigned int, MaterialProperties> >;
}

MaterialPropertyStorage::~MaterialPropertyStorage()
{
  delete _props_elem;
  delete _props_elem_old;
  delete _props_elem_older;
}

void
MaterialPropertyStorage::releaseProperties()
{
  HashMap<unsigned int, HashMap<unsigned int, MaterialProperties> >::iterator i;
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
MaterialPropertyStorage::initStatefulProps(MaterialData & material_data, std::vector<Material *> & mats, unsigned int n_qpoints, const Elem & elem, unsigned int side/* = 0*/)
{
  // NOTE: since materials are storing their computed properties in MaterialData class, we need to
  // juggle the memory between MaterialData and MaterialProperyStorage classes

  unsigned int elem_id = elem.id();

  material_data.size(n_qpoints);

  if (props()[elem_id][side].size() == 0) props()[elem_id][side].resize(material_data.props().size());
  if (propsOld()[elem_id][side].size() == 0) propsOld()[elem_id][side].resize(material_data.propsOld().size());
  if (propsOlder()[elem_id][side].size() == 0) propsOlder()[elem_id][side].resize(material_data.propsOlder().size());

  // init properties (allocate memory. etc)
  for (std::set<unsigned int>::const_iterator it = _stateful_props.begin(); it != _stateful_props.end(); ++it)
  {
    unsigned int prop_id = *it;
    // duplicate the stateful property in property storage (all three states - we will reuse the allocated memory there)
    // also allocating the right amount of memory, so we do not have to resize, etc.
    if (props()[elem_id][side][prop_id] == NULL) props()[elem_id][side][prop_id] = material_data.props()[prop_id]->init(n_qpoints);
    if (propsOld()[elem_id][side][prop_id] == NULL) propsOld()[elem_id][side][prop_id] = material_data.propsOld()[prop_id]->init(n_qpoints);
    if (hasOlderProperties())
      if (propsOlder()[elem_id][side][prop_id] == NULL) propsOlder()[elem_id][side][prop_id] = material_data.propsOld()[prop_id]->init(n_qpoints);
  }
  // copy from storage to material data
  swap(material_data, elem, side);
  // run custom init on properties
  for (std::vector<Material *>::iterator it = mats.begin(); it != mats.end(); ++it)
    (*it)->initStatefulProperties(n_qpoints);
  swapBack(material_data, elem, side);
}

void
MaterialPropertyStorage::shift()
{
  if (_has_older_prop)
  {
    // shift the properties back in time and reuse older for current (save reallocations etc.)
    HashMap<unsigned int, HashMap<unsigned int, MaterialProperties> > * tmp = _props_elem_older;
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

  unsigned int elem_id = elem.id();
  shallowCopyData(_stateful_props, material_data.props(), props()[elem_id][side]);
  shallowCopyData(_stateful_props, material_data.propsOld(), propsOld()[elem_id][side]);
  if (hasOlderProperties())
    shallowCopyData(_stateful_props, material_data.propsOlder(), propsOlder()[elem_id][side]);
}

void
MaterialPropertyStorage::swapBack(MaterialData & material_data, const Elem & elem, unsigned int side)
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

  unsigned int elem_id = elem.id();
  shallowCopyData(_stateful_props, props()[elem_id][side], material_data.props());
  shallowCopyData(_stateful_props, propsOld()[elem_id][side], material_data.propsOld());
  if (hasOlderProperties())
    shallowCopyData(_stateful_props, propsOlder()[elem_id][side], material_data.propsOlder());
}

void
MaterialPropertyStorage::addProperty (unsigned int prop_id, const std::string & prop_name)
{
  _prop_names[prop_id] = prop_name;
}

void
MaterialPropertyStorage::addPropertyOld (unsigned int prop_id, const std::string & prop_name)
{
  addProperty(prop_id, prop_name);
  _has_stateful_props = true;
  _stateful_props.insert(prop_id);
}

void
MaterialPropertyStorage::addPropertyOlder (unsigned int prop_id, const std::string & prop_name)
{
  addProperty(prop_id, prop_name);
  _has_stateful_props = true;
  _has_older_prop = true;
  _stateful_props.insert(prop_id);
}
