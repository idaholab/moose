/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "BoundaryFluxBase.h"

// Static mutex definition
Threads::spin_mutex BoundaryFluxBase::_mutex;

template <>
InputParameters
validParams<BoundaryFluxBase>()
{
  InputParameters params = validParams<GeneralUserObject>();
  return params;
}

BoundaryFluxBase::BoundaryFluxBase(const InputParameters & parameters)
  : GeneralUserObject(parameters)
{
  _flux.resize(libMesh::n_threads());
  _jac1.resize(libMesh::n_threads());
}

void
BoundaryFluxBase::initialize()
{
  _cached_elem_id = 0;
  _cached_side_id = libMesh::invalid_uint;
}

void
BoundaryFluxBase::execute()
{
}

void
BoundaryFluxBase::finalize()
{
}

const std::vector<Real> &
BoundaryFluxBase::getFlux(unsigned int iside,
                          dof_id_type ielem,
                          const std::vector<Real> & uvec1,
                          const RealVectorValue & dwave,
                          THREAD_ID tid) const
{
  Threads::spin_mutex::scoped_lock lock(_mutex);
  if (_cached_elem_id != ielem || _cached_side_id != iside)
  {
    _cached_elem_id = ielem;
    _cached_side_id = iside;

    calcFlux(iside, ielem, uvec1, dwave, _flux[tid]);
  }
  return _flux[tid];
}

const DenseMatrix<Real> &
BoundaryFluxBase::getJacobian(unsigned int iside,
                              dof_id_type ielem,
                              const std::vector<Real> & uvec1,
                              const RealVectorValue & dwave,
                              THREAD_ID tid) const
{
  Threads::spin_mutex::scoped_lock lock(_mutex);
  if (_cached_elem_id != ielem || _cached_side_id != iside)
  {
    _cached_elem_id = ielem;
    _cached_side_id = iside;

    calcJacobian(iside, ielem, uvec1, dwave, _jac1[tid]);
  }
  return _jac1[tid];
}
