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
    _n_qpoints(0)
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
  if (_storage.hasStatefulProperties())
  {
    _storage.swapBack(*this, elem, side);
    _swapped = false;
  }
}

bool
MaterialData::have_property_name(const std::string & prop_name) const
{
  return _storage.hasProperty(prop_name);
}

bool
MaterialData::have_property_name_old(const std::string & prop_name) const
{
  return _storage.hasProperty(prop_name);
}

bool
MaterialData::have_property_name_older(const std::string & prop_name) const
{
  return _storage.hasProperty(prop_name);
}

bool
MaterialData::isSwapped()
{
  return _swapped;
}
