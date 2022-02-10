//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADBoundaryFluxBase.h"

InputParameters
ADBoundaryFluxBase::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  return params;
}

ADBoundaryFluxBase::ADBoundaryFluxBase(const InputParameters & parameters)
  : ThreadedGeneralUserObject(parameters),
    _cached_flux_elem_id(libMesh::invalid_uint),
    _cached_flux_side_id(libMesh::invalid_uint)
{
}

void
ADBoundaryFluxBase::initialize()
{
  _cached_flux_elem_id = libMesh::invalid_uint;
  _cached_flux_side_id = libMesh::invalid_uint;
}

void
ADBoundaryFluxBase::execute()
{
}

void
ADBoundaryFluxBase::finalize()
{
}

void
ADBoundaryFluxBase::threadJoin(const UserObject &)
{
}

const std::vector<ADReal> &
ADBoundaryFluxBase::getFlux(unsigned int iside,
                            dof_id_type ielem,
                            const std::vector<ADReal> & uvec1,
                            const RealVectorValue & dwave) const
{
  if (_cached_flux_elem_id != ielem || _cached_flux_side_id != iside)
  {
    _cached_flux_elem_id = ielem;
    _cached_flux_side_id = iside;

    calcFlux(iside, ielem, uvec1, dwave, _flux);
  }
  return _flux;
}
