//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialData.h"
#include "Material.h"

MaterialData::MaterialData(MaterialPropertyStorage & storage)
  : _storage(storage), _n_qpoints(0), _swapped(false)
{
}

MaterialData::~MaterialData() { release(); }

void
MaterialData::release()
{
  _props.destroy();
  _props_old.destroy();
  _props_older.destroy();
}

void
MaterialData::resize(unsigned int n_qpoints)
{
  if (n_qpoints == _n_qpoints)
    return;

  _props.resizeItems(n_qpoints);
  // if there are stateful material properties in the system, also resize
  // storage for old and older material properties
  if (_storage.hasStatefulProperties())
    _props_old.resizeItems(n_qpoints);
  if (_storage.hasOlderProperties())
    _props_older.resizeItems(n_qpoints);
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
MaterialData::swap(const Elem & elem, unsigned int side /* = 0*/)
{
  if (_swapped || (!_storage.hasStatefulProperties() && !_use_cache))
    return;

  _storage.swap(*this, elem, side);
  _swapped = true;
  _swapped_elem = &elem;
  _swapped_side = side;
}

void
MaterialData::reinit(const std::vector<std::shared_ptr<Material>> & mats)
{
  for (const auto & mat : mats)
  {
    MaterialCacheKey key(mat.get(), _swapped_elem, _swapped_side);
    if (!_use_cache || _cache.count(key) == 0)
    {
      mat->computeProperties();
      std::cout << "recomputing value for mat=" << mat->name()
                << ", elem=" << (_swapped_elem ? _swapped_elem->id() : 0) << "\n";
      if (_use_cache)
        _cache[key] = true;
    }
    else
      std::cout << "reusing value for mat=" << mat->name()
                << ", elem=" << (_swapped_elem ? _swapped_elem->id() : 0) << "\n";
  }
}

void
MaterialData::reset(const std::vector<std::shared_ptr<Material>> & mats)
{
  for (const auto & mat : mats)
    mat->resetProperties();
}

void
MaterialData::swapBack(const Elem & elem, unsigned int side /* = 0*/)
{
  if (!_swapped)
    return;

  _storage.swapBack(*this, elem, side);
  _swapped = false;
  _swapped_elem = nullptr;
  _swapped_side = 0;
}

bool
MaterialData::isSwapped()
{
  return _swapped;
}
