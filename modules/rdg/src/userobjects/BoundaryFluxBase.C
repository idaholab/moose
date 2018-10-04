//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundaryFluxBase.h"

template <>
InputParameters
validParams<BoundaryFluxBase>()
{
  InputParameters params = validParams<GeneralUserObject>();
  return params;
}

BoundaryFluxBase::BoundaryFluxBase(const InputParameters & parameters)
  : ThreadedGeneralUserObject(parameters)
{
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
  if (_cached_elem_id != ielem || _cached_side_id != iside)
  {
    _cached_elem_id = ielem;
    _cached_side_id = iside;

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
  if (_cached_elem_id != ielem || _cached_side_id != iside)
  {
    _cached_elem_id = ielem;
    _cached_side_id = iside;

    calcJacobian(iside, ielem, uvec1, dwave, _jac1);
  }
  return _jac1;
}
