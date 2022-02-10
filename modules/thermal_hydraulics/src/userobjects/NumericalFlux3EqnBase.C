//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NumericalFlux3EqnBase.h"

InputParameters
NumericalFlux3EqnBase::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  return params;
}

NumericalFlux3EqnBase::NumericalFlux3EqnBase(const InputParameters & parameters)
  : ThreadedGeneralUserObject(parameters),

    _cached_flux_elem_id(libMesh::invalid_uint),
    _cached_flux_side_id(libMesh::invalid_uint),
    _cached_jacobian_elem_id(libMesh::invalid_uint),
    _cached_jacobian_side_id(libMesh::invalid_uint),

    _last_region_index(0)
{
}

void
NumericalFlux3EqnBase::initialize()
{
  _cached_flux_elem_id = libMesh::invalid_uint;
  _cached_flux_side_id = libMesh::invalid_uint;
  _cached_jacobian_elem_id = libMesh::invalid_uint;
  _cached_jacobian_side_id = libMesh::invalid_uint;
}

void
NumericalFlux3EqnBase::execute()
{
}

void
NumericalFlux3EqnBase::finalize()
{
}

void
NumericalFlux3EqnBase::threadJoin(const UserObject &)
{
}

const std::vector<Real> &
NumericalFlux3EqnBase::getFlux(const unsigned int iside,
                               const dof_id_type ielem,
                               bool res_side_is_left,
                               const std::vector<Real> & UL,
                               const std::vector<Real> & UR,
                               const Real & nLR_dot_d) const
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

const DenseMatrix<Real> &
NumericalFlux3EqnBase::getJacobian(bool res_side_is_left,
                                   bool jac_side_is_left,
                                   const unsigned int iside,
                                   const dof_id_type ielem,
                                   const std::vector<Real> & UL,
                                   const std::vector<Real> & UR,
                                   const Real & nLR_dot_d) const
{
  if (_cached_jacobian_elem_id != ielem || _cached_jacobian_side_id != iside)
  {
    _cached_jacobian_elem_id = ielem;
    _cached_jacobian_side_id = iside;

    calcJacobian(UL, UR, nLR_dot_d, _dFL_dUL, _dFL_dUR, _dFR_dUL, _dFR_dUR);
  }

  if (res_side_is_left)
  {
    if (jac_side_is_left)
      return _dFL_dUL;
    else
      return _dFL_dUR;
  }
  else
  {
    if (jac_side_is_left)
      return _dFR_dUL;
    else
      return _dFR_dUR;
  }
}
