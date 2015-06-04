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

MaterialData::MaterialData(MaterialPropertyStorage & storage) :
    _storage(storage),
    _n_qpoints(0),
    _swapped(false)
{
}

MaterialData::~MaterialData()
{
  release();
}

void
MaterialData::release()
{
  _props.destroy();
  _props_old.destroy();
  _props_older.destroy();
}

void
MaterialData::size(unsigned int n_qpoints)
{
  _props.resizeItems(n_qpoints);
  // if there are stateful material properties in the system, also resize
  // storage for old and older material properties
  if (_storage.hasStatefulProperties())
  {
    _props_old.resizeItems(n_qpoints);

    if (_storage.hasOlderProperties())
      _props_older.resizeItems(n_qpoints);
  }
  _n_qpoints = n_qpoints;
}

unsigned int
MaterialData::nQPoints()
{
  return _n_qpoints;
}

void
MaterialData::copy(const Elem & elem_to, const Elem & elem_from, unsigned int side)
{
  _storage.copy(*this, elem_to, elem_from, side, _n_qpoints);
}

void
MaterialData::swap(const Elem & elem, unsigned int side/* = 0*/)
{
  if (_storage.hasStatefulProperties())
  {
    _storage.swap(*this, elem, side);
    _swapped = true;
  }
}

void
MaterialData::reinit(std::vector<Material *> & mats)
{
  for (std::vector<Material *>::iterator it = mats.begin(); it != mats.end(); ++it)
    (*it)->computeProperties();
}

void
MaterialData::swapBack(const Elem & elem, unsigned int side/* = 0*/)
{
  if (_swapped && _storage.hasStatefulProperties())
  {
    _storage.swapBack(*this, elem, side);
    _swapped = false;
  }
}

bool
MaterialData::isSwapped()
{
  return _swapped;
}

void
MaterialData::registerMatPropWithMaterial(const std::string & name, Material * mat)
{
  unsigned int prop_id = getPropertyId(name);
  _material_to_prop_id[mat].insert(prop_id);
}

void
MaterialData::reinitMatPropWithMaterial(const std::vector<Material *> & mats)
{
  _active_prop_id_to_material.clear();
  for (std::vector<Material *>::const_iterator mat_it = mats.begin(); mat_it != mats.end(); ++mat_it)
  {
    const std::set<unsigned int> & ids = _material_to_prop_id[*mat_it];
    for (std::set<unsigned int>::const_iterator id_it = ids.begin(); id_it != ids.end(); ++id_it)
      _active_prop_id_to_material.insert(std::pair<unsigned int, Material *>(*id_it, *mat_it));
  }
}


Material *
MaterialData::getActiveMaterial(const unsigned int & prop_id)
{
  std::map<unsigned int, Material *>::const_iterator iter = _active_prop_id_to_material.find(prop_id);
  if (iter == _active_prop_id_to_material.end())
    mooseError("The material property, '" << _storage.getPropertyName(prop_id) << "', is not defined on an active Material.");
  return iter->second;
}


unsigned int
MaterialData::getPropertyId(const std::string & prop_name)
{
  return _storage.getPropertyId(prop_name);
}
