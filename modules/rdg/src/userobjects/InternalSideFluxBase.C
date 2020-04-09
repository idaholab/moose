//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InternalSideFluxBase.h"

InputParameters
InternalSideFluxBase::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("A base class for computing and caching internal side flux.");
  return params;
}

InternalSideFluxBase::InternalSideFluxBase(const InputParameters & parameters)
  : ThreadedGeneralUserObject(parameters),
    _cached_flux_elem_id(libMesh::invalid_uint),
    _cached_flux_neig_id(libMesh::invalid_uint),
    _cached_jacobian_elem_id(libMesh::invalid_uint),
    _cached_jacobian_neig_id(libMesh::invalid_uint)
{
}

void
InternalSideFluxBase::initialize()
{
  _cached_flux_elem_id = libMesh::invalid_uint;
  _cached_flux_neig_id = libMesh::invalid_uint;
  _cached_jacobian_elem_id = libMesh::invalid_uint;
  _cached_jacobian_neig_id = libMesh::invalid_uint;
}

void
InternalSideFluxBase::execute()
{
}

void
InternalSideFluxBase::finalize()
{
}

void
InternalSideFluxBase::threadJoin(const UserObject &)
{
}

const std::vector<Real> &
InternalSideFluxBase::getFlux(unsigned int iside,
                              dof_id_type ielem,
                              dof_id_type ineig,
                              const std::vector<Real> & uvec1,
                              const std::vector<Real> & uvec2,
                              const RealVectorValue & dwave) const
{
  if (_cached_flux_elem_id != ielem || _cached_flux_neig_id != ineig)
  {
    _cached_flux_elem_id = ielem;
    _cached_flux_neig_id = ineig;

    calcFlux(iside, ielem, ineig, uvec1, uvec2, dwave, _flux);
  }
  return _flux;
}

const DenseMatrix<Real> &
InternalSideFluxBase::getJacobian(Moose::DGResidualType type,
                                  unsigned int iside,
                                  dof_id_type ielem,
                                  dof_id_type ineig,
                                  const std::vector<Real> & uvec1,
                                  const std::vector<Real> & uvec2,
                                  const RealVectorValue & dwave) const
{
  if (_cached_jacobian_elem_id != ielem || _cached_jacobian_neig_id != ineig)
  {
    _cached_jacobian_elem_id = ielem;
    _cached_jacobian_neig_id = ineig;

    calcJacobian(iside, ielem, ineig, uvec1, uvec2, dwave, _jac1, _jac2);
  }

  if (type == Moose::Element)
    return _jac1;
  else
    return _jac2;
}
