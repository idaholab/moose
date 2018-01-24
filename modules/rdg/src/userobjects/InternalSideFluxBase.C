//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InternalSideFluxBase.h"

// Static mutex definition
Threads::spin_mutex InternalSideFluxBase::_mutex;

template <>
InputParameters
validParams<InternalSideFluxBase>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("A base class for computing and caching internal side flux.");
  return params;
}

InternalSideFluxBase::InternalSideFluxBase(const InputParameters & parameters)
  : GeneralUserObject(parameters)
{
  _flux.resize(libMesh::n_threads());
  _jac1.resize(libMesh::n_threads());
  _jac2.resize(libMesh::n_threads());
}

void
InternalSideFluxBase::initialize()
{
  _cached_elem_id = 0;
  _cached_neig_id = 0;
}

void
InternalSideFluxBase::execute()
{
}

void
InternalSideFluxBase::finalize()
{
}

const std::vector<Real> &
InternalSideFluxBase::getFlux(unsigned int iside,
                              dof_id_type ielem,
                              dof_id_type ineig,
                              const std::vector<Real> & uvec1,
                              const std::vector<Real> & uvec2,
                              const RealVectorValue & dwave,
                              THREAD_ID tid) const
{
  Threads::spin_mutex::scoped_lock lock(_mutex);
  if (_cached_elem_id != ielem || _cached_neig_id != ineig)
  {
    _cached_elem_id = ielem;
    _cached_neig_id = ineig;

    calcFlux(iside, ielem, ineig, uvec1, uvec2, dwave, _flux[tid]);
  }
  return _flux[tid];
}

const DenseMatrix<Real> &
InternalSideFluxBase::getJacobian(Moose::DGResidualType type,
                                  unsigned int iside,
                                  dof_id_type ielem,
                                  dof_id_type ineig,
                                  const std::vector<Real> & uvec1,
                                  const std::vector<Real> & uvec2,
                                  const RealVectorValue & dwave,
                                  THREAD_ID tid) const
{
  Threads::spin_mutex::scoped_lock lock(_mutex);
  if (_cached_elem_id != ielem || _cached_neig_id != ineig)
  {
    _cached_elem_id = ielem;
    _cached_neig_id = ineig;

    calcJacobian(iside, ielem, ineig, uvec1, uvec2, dwave, _jac1[tid], _jac2[tid]);
  }

  if (type == Moose::Element)
    return _jac1[tid];
  else
    return _jac2[tid];
}
