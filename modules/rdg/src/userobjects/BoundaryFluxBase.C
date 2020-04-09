//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundaryFluxBase.h"

InputParameters
BoundaryFluxBase::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  return params;
}

BoundaryFluxBase::BoundaryFluxBase(const InputParameters & parameters)
  : ThreadedGeneralUserObject(parameters),
    _cached_flux_elem_id(libMesh::invalid_uint),
    _cached_flux_side_id(libMesh::invalid_uint),
    _cached_jacobian_elem_id(libMesh::invalid_uint),
    _cached_jacobian_side_id(libMesh::invalid_uint)
{
}

void
BoundaryFluxBase::initialize()
{
  _cached_flux_elem_id = libMesh::invalid_uint;
  _cached_flux_side_id = libMesh::invalid_uint;
  _cached_jacobian_elem_id = libMesh::invalid_uint;
  _cached_jacobian_side_id = libMesh::invalid_uint;
}

void
BoundaryFluxBase::execute()
{
}

void
BoundaryFluxBase::finalize()
{
}

void
BoundaryFluxBase::threadJoin(const UserObject &)
{
}

const std::vector<Real> &
BoundaryFluxBase::getFlux(unsigned int iside,
                          dof_id_type ielem,
                          const std::vector<Real> & uvec1,
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

const DenseMatrix<Real> &
BoundaryFluxBase::getJacobian(unsigned int iside,
                              dof_id_type ielem,
                              const std::vector<Real> & uvec1,
                              const RealVectorValue & dwave) const
{
  if (_cached_jacobian_elem_id != ielem || _cached_jacobian_side_id != iside)
  {
    _cached_jacobian_elem_id = ielem;
    _cached_jacobian_side_id = iside;

    calcJacobian(iside, ielem, uvec1, dwave, _jac1);
  }
  return _jac1;
}
