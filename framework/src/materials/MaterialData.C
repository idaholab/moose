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

#include "MaterialData.h"
#include "Material.h"

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


MaterialData::MaterialData(MaterialPropertyStorage & storage) :
    _storage(storage),
    _sized(false)
{
}

MaterialData::~MaterialData()
{
  for (MaterialProperties::iterator it = _props.begin(); it != _props.end(); ++it)
    delete *it;
  for (MaterialProperties::iterator it = _props_old.begin(); it != _props_old.end(); ++it)
    delete *it;
  for (MaterialProperties::iterator it = _props_older.begin(); it != _props_older.end(); ++it)
    delete *it;
}

void
MaterialData::size(unsigned int n_qpoints)
{
  for (MaterialProperties::iterator it = _props.begin(); it != _props.end(); ++it)
  {
    if (*it != NULL)
      (*it)->resize(n_qpoints);
  }

  if (_storage.hasStatefulProperties())
  {
    for (MaterialProperties::iterator it = _props_old.begin(); it != _props_old.end(); ++it)
      if (*it != NULL)
        (*it)->resize(n_qpoints);

    if (_storage.hasStatefulProperties())
      for (MaterialProperties::iterator it = _props_older.begin(); it != _props_older.end(); ++it)
        if (*it != NULL)
          (*it)->resize(n_qpoints);
  }
  _sized = true;
}

void
MaterialData::initStatefulProps(std::vector<Material *> & mats, unsigned int n_qpoints, const Elem & elem, unsigned int side/* = 0*/)
{
  unsigned int elem_id = elem.id();

  size(n_qpoints);

  if (_storage.props()[elem_id][side].size() == 0) _storage.props()[elem_id][side].resize(_props.size());
  if (_storage.propsOld()[elem_id][side].size() == 0) _storage.propsOld()[elem_id][side].resize(_props_old.size());
  if (_storage.propsOlder()[elem_id][side].size() == 0) _storage.propsOlder()[elem_id][side].resize(_props_older.size());

  // init properties (allocate memory. etc)
  for (std::set<unsigned int>::const_iterator it = _stateful_props.begin(); it != _stateful_props.end(); ++it)
  {
    unsigned int prop_id = *it;
    // duplicate the stateful property in property storage (all three states - we will reuse the allocated memory there)
    // also allocating the right amount of memory, so we do not have to resize, etc.
    if (_storage.props()[elem_id][side][prop_id] == NULL) _storage.props()[elem_id][side][prop_id] = _props[prop_id]->init(n_qpoints);
    if (_storage.propsOld()[elem_id][side][prop_id] == NULL) _storage.propsOld()[elem_id][side][prop_id] = _props_old[prop_id]->init(n_qpoints);
    if (_storage.hasOlderProperties())
      if (_storage.propsOlder()[elem_id][side][prop_id] == NULL) _storage.propsOlder()[elem_id][side][prop_id] = _props_old[prop_id]->init(n_qpoints);
  }
  // copy from storage to material data
  shallowCopyData(_stateful_props, _props, _storage.props()[elem_id][side]);
  shallowCopyData(_stateful_props, _props_old, _storage.propsOld()[elem_id][side]);
  if (_storage.hasOlderProperties())
    shallowCopyData(_stateful_props, _props_older, _storage.propsOlder()[elem_id][side]);
  // run custom init on properties
  for (std::vector<Material *>::iterator it = mats.begin(); it != mats.end(); ++it)
    (*it)->initStatefulProperties(n_qpoints);
  // copy from material_data to storage
  shallowCopyData(_stateful_props, _storage.props()[elem_id][side], _props);
  shallowCopyData(_stateful_props, _storage.propsOld()[elem_id][side], _props_old);
  if (_storage.hasOlderProperties())
    shallowCopyData(_stateful_props, _storage.propsOlder()[elem_id][side], _props_older);
}

void
MaterialData::reinit(std::vector<Material *> & mats, unsigned int n_qpoints, const Elem & elem, unsigned int side/* = 0*/)
{
  unsigned int elem_id = elem.id();

  if (!_sized)
    size(n_qpoints);

  if (_storage.hasStatefulProperties())
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

    shallowCopyData(_stateful_props, _props, _storage.props()[elem_id][side]);
    shallowCopyData(_stateful_props, _props_old, _storage.propsOld()[elem_id][side]);
    if (_storage.hasOlderProperties())
      shallowCopyData(_stateful_props, _props_older, _storage.propsOlder()[elem_id][side]);
  }

  // TODO: use the dep resolver to iterate over this vector in the right order
  // iterate over materials and compute their properties
  for (std::vector<Material *>::iterator it = mats.begin(); it != mats.end(); ++it)
    (*it)->computeProperties();

  if (_storage.hasStatefulProperties())
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

    shallowCopyData(_stateful_props, _storage.props()[elem_id][side], _props);
    shallowCopyData(_stateful_props, _storage.propsOld()[elem_id][side], _props_old);
    if (_storage.hasOlderProperties())
      shallowCopyData(_stateful_props, _storage.propsOlder()[elem_id][side], _props_older);
  }
}

bool
MaterialData::have_property_name(const std::string & prop_name) const
{
  std::map<std::string, unsigned int>::const_iterator it = _prop_ids.find(prop_name);
  return (it != _prop_ids.end());
}

bool
MaterialData::have_property_name_old(const std::string & prop_name) const
{
  return have_property_name(prop_name);
}

bool
MaterialData::have_property_name_older(const std::string & prop_name) const
{
  return have_property_name(prop_name);
}
