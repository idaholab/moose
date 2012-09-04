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
  unsigned int j = 0;
  for (MaterialProperties::iterator it = _props.begin(); it != _props.end(); ++it, j++)
  {
    if (*it != NULL)
      (*it)->resize(n_qpoints);
  }

  // if there are stateful material properties in the system, also resize
  // storage for old and older material properties
  if (_storage.hasStatefulProperties())
  {
    for (MaterialProperties::iterator it = _props_old.begin(); it != _props_old.end(); ++it)
      if (*it != NULL)
        (*it)->resize(n_qpoints);

    if (_storage.hasOlderProperties())
      for (MaterialProperties::iterator it = _props_older.begin(); it != _props_older.end(); ++it)
        if (*it != NULL)
          (*it)->resize(n_qpoints);
  }
  _sized = true;
}

void
MaterialData::reinit(std::vector<Material *> & mats, unsigned int n_qpoints, const Elem & elem, unsigned int side/* = 0*/)
{
  // FIXME: if there are elements with different number of quadrature points and the one with smaller
  // number gets hit first, we will fail with out-of-bound error, since we set the resize flag to true,
  // but we need more space to store the new properties.  Thus, we need to be smarter here with invalidating
  // the _size flag.
  if (!_sized)
    size(n_qpoints);

  if (_storage.hasStatefulProperties())
    _storage.swap(*this, elem, side);

  for (std::vector<Material *>::iterator it = mats.begin(); it != mats.end(); ++it)
    (*it)->computeProperties();

  if (_storage.hasStatefulProperties())
    _storage.swapBack(*this, elem, side);
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
