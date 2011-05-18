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

void shallowCopyData(const std::set<std::string> & names,
                     MaterialProperties & data,
                     MaterialProperties & data_from)
{
  for (std::set<std::string>::const_iterator it = names.begin(); it != names.end(); ++it)
  {
    std::string name = *it;
    if (data[name] != NULL && data_from[name] != NULL)
      data[name]->shallowCopy(data_from[name]);
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
    delete it->second;
  for (MaterialProperties::iterator it = _props_old.begin(); it != _props_old.end(); ++it)
    delete it->second;
  for (MaterialProperties::iterator it = _props_older.begin(); it != _props_older.end(); ++it)
    delete it->second;
}

void
MaterialData::size(unsigned int n_qpoints)
{
  for (MaterialProperties::iterator it = _props.begin(); it != _props.end(); ++it)
    if (it->second != NULL)
      it->second->resize(n_qpoints);

  if (_storage.hasStatefulProperties())
  {
    for (MaterialProperties::iterator it = _props_old.begin(); it != _props_old.end(); ++it)
      if (it->second != NULL)
        it->second->resize(n_qpoints);

    if (_storage.hasStatefulProperties())
      for (MaterialProperties::iterator it = _props_older.begin(); it != _props_older.end(); ++it)
        if (it->second != NULL)
          it->second->resize(n_qpoints);
  }
  _sized = true;
}

void
MaterialData::initStatefulProps(std::vector<Material *> & mats, unsigned int n_qpoints, const Elem & /*elem*/, unsigned int /*side = 0*/)
{
  size(n_qpoints);
  for (std::vector<Material *>::iterator it = mats.begin(); it != mats.end(); ++it)
    (*it)->initStatefulProperties();
}

void
MaterialData::reinit(std::vector<Material *> & mats, unsigned int n_qpoints, const Elem & elem, unsigned int side/* = 0*/)
{
  unsigned int elem_id = elem.id();

  if (!_sized)
  {
    size(n_qpoints);
    _sized = true;
  }

  if (_storage.hasStatefulProperties())
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

    for (std::set<std::string>::const_iterator it = _stateful_props.begin(); it != _stateful_props.end(); ++it)
    {
      std::string name = *it;
      // duplicate the stateful property in property storage (all three states - we will reuse the allocated memory there)
      // also allocating the right amount of memory, so we do not have to resize, etc.
      if (_storage.props()[elem_id][side][name] == NULL) _storage.props()[elem_id][side][name] = _props[name]->init(n_qpoints);
      if (_storage.propsOld()[elem_id][side][name] == NULL) _storage.propsOld()[elem_id][side][name] = _props_old[name]->init(n_qpoints);
      if (_storage.hasOlderProperties())
        if (_storage.propsOlder()[elem_id][side][name] == NULL) _storage.propsOlder()[elem_id][side][name] = _props_old[name]->init(n_qpoints);
    }

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
