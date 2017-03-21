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
  if (!_storage.hasStatefulProperties() || _swapped)
    return;

  _storage.swap(*this, elem, side);
  _swapped = true;
}

void
MaterialData::reinit(const std::vector<std::shared_ptr<Material>> & mats)
{
  for (const auto & mat : mats)
    mat->computeProperties();
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
