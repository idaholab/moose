//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RDGFluxBase.h"

// Static mutex definition
Threads::spin_mutex RDGFluxBase::_mutex;

template <>
InputParameters
validParams<RDGFluxBase>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription(
      "Abstract base class for computing and caching internal or boundary fluxes for RDG");
  return params;
}

RDGFluxBase::RDGFluxBase(const InputParameters & parameters) : GeneralUserObject(parameters)
{
  _flux.resize(libMesh::n_threads());
  _jac1.resize(libMesh::n_threads());
  _jac2.resize(libMesh::n_threads());
}

void
RDGFluxBase::initialize()
{
  _cached_elem_id = libMesh::invalid_uint;
  _cached_side_id = libMesh::invalid_uint;
}

void
RDGFluxBase::execute()
{
}

void
RDGFluxBase::finalize()
{
}

const std::vector<Real> &
RDGFluxBase::getFlux(const unsigned int iside,
                     const dof_id_type ielem,
                     const std::vector<Real> & uvec1,
                     const std::vector<Real> & uvec2,
                     const RealVectorValue & normal,
                     THREAD_ID tid) const
{
  Threads::spin_mutex::scoped_lock lock(_mutex);
  if (_cached_elem_id != ielem || _cached_side_id != iside)
  {
    _cached_elem_id = ielem;
    _cached_side_id = iside;

    calcFlux(uvec1, uvec2, normal, _flux[tid]);
  }
  return _flux[tid];
}

const DenseMatrix<Real> &
RDGFluxBase::getJacobian(const bool get_first_jacobian,
                         const unsigned int iside,
                         const dof_id_type ielem,
                         const std::vector<Real> & uvec1,
                         const std::vector<Real> & uvec2,
                         const RealVectorValue & normal,
                         THREAD_ID tid) const
{
  Threads::spin_mutex::scoped_lock lock(_mutex);
  if (_cached_elem_id != ielem || _cached_side_id != iside)
  {
    _cached_elem_id = ielem;
    _cached_side_id = iside;

    calcJacobian(uvec1, uvec2, normal, _jac1[tid], _jac2[tid]);
  }

  if (get_first_jacobian)
    return _jac1[tid];
  else
    return _jac2[tid];
}
