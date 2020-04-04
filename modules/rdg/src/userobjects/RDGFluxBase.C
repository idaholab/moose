//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RDGFluxBase.h"

InputParameters
RDGFluxBase::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription(
      "Abstract base class for computing and caching internal or boundary fluxes for RDG");
  return params;
}

RDGFluxBase::RDGFluxBase(const InputParameters & parameters)
  : ThreadedGeneralUserObject(parameters),
    _cached_flux_elem_id(libMesh::invalid_uint),
    _cached_flux_side_id(libMesh::invalid_uint),
    _cached_jacobian_elem_id(libMesh::invalid_uint),
    _cached_jacobian_side_id(libMesh::invalid_uint)
{
}

void
RDGFluxBase::initialize()
{
  _cached_flux_elem_id = libMesh::invalid_uint;
  _cached_flux_side_id = libMesh::invalid_uint;
  _cached_jacobian_elem_id = libMesh::invalid_uint;
  _cached_jacobian_side_id = libMesh::invalid_uint;
}

void
RDGFluxBase::execute()
{
}

void
RDGFluxBase::finalize()
{
}

void
RDGFluxBase::threadJoin(const UserObject &)
{
}

const std::vector<Real> &
RDGFluxBase::getFlux(const unsigned int iside,
                     const dof_id_type ielem,
                     const std::vector<Real> & uvec1,
                     const std::vector<Real> & uvec2,
                     const RealVectorValue & normal) const
{
  if (_cached_flux_elem_id != ielem || _cached_flux_side_id != iside)
  {
    _cached_flux_elem_id = ielem;
    _cached_flux_side_id = iside;

    calcFlux(uvec1, uvec2, normal, _flux);
  }
  return _flux;
}

const DenseMatrix<Real> &
RDGFluxBase::getJacobian(const bool get_first_jacobian,
                         const unsigned int iside,
                         const dof_id_type ielem,
                         const std::vector<Real> & uvec1,
                         const std::vector<Real> & uvec2,
                         const RealVectorValue & normal) const
{
  if (_cached_jacobian_elem_id != ielem || _cached_jacobian_side_id != iside)
  {
    _cached_jacobian_elem_id = ielem;
    _cached_jacobian_side_id = iside;

    calcJacobian(uvec1, uvec2, normal, _jac1, _jac2);
  }

  if (get_first_jacobian)
    return _jac1;
  else
    return _jac2;
}
