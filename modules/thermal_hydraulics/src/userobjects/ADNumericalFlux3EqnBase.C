//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADNumericalFlux3EqnBase.h"

InputParameters
ADNumericalFlux3EqnBase::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  return params;
}

ADNumericalFlux3EqnBase::ADNumericalFlux3EqnBase(const InputParameters & parameters)
  : ThreadedGeneralUserObject(parameters),

    _cached_flux_elem_id(libMesh::invalid_uint),
    _cached_flux_side_id(libMesh::invalid_uint),

    _last_region_index(0)
{
}

void
ADNumericalFlux3EqnBase::initialize()
{
  _cached_flux_elem_id = libMesh::invalid_uint;
  _cached_flux_side_id = libMesh::invalid_uint;
}

void
ADNumericalFlux3EqnBase::execute()
{
}

void
ADNumericalFlux3EqnBase::finalize()
{
}

void
ADNumericalFlux3EqnBase::threadJoin(const UserObject &)
{
}

const std::vector<ADReal> &
ADNumericalFlux3EqnBase::getFlux(const unsigned int iside,
                                 const dof_id_type ielem,
                                 bool res_side_is_left,
                                 const std::vector<ADReal> & UL,
                                 const std::vector<ADReal> & UR,
                                 const ADReal & nLR_dot_d) const
{
  if (_cached_flux_elem_id != ielem || _cached_flux_side_id != iside)
  {
    _cached_flux_elem_id = ielem;
    _cached_flux_side_id = iside;

    calcFlux(UL, UR, nLR_dot_d, _FL, _FR);
  }

  if (res_side_is_left)
    return _FL;
  else
    return _FR;
}
