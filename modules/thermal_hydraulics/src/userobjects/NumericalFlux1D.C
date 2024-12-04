//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NumericalFlux1D.h"
#include "THMUtils.h"

InputParameters
NumericalFlux1D::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  return params;
}

NumericalFlux1D::NumericalFlux1D(const InputParameters & parameters)
  : ThreadedGeneralUserObject(parameters),

    _cached_flux_elem_id(libMesh::invalid_uint),
    _cached_flux_side_id(libMesh::invalid_uint),

    _last_region_index(0)
{
}

void
NumericalFlux1D::initialize()
{
  _cached_flux_elem_id = libMesh::invalid_uint;
  _cached_flux_side_id = libMesh::invalid_uint;
}

void
NumericalFlux1D::execute()
{
}

void
NumericalFlux1D::finalize()
{
}

void
NumericalFlux1D::threadJoin(const UserObject &)
{
}

const std::vector<ADReal> &
NumericalFlux1D::getFlux(const unsigned int iside,
                         const dof_id_type ielem,
                         bool res_side_is_left,
                         const std::vector<ADReal> & UL_1d,
                         const std::vector<ADReal> & UR_1d,
                         const ADReal & nLR_dot_d) const
{
  if (_cached_flux_elem_id != ielem || _cached_flux_side_id != iside)
  {
    _cached_flux_elem_id = ielem;
    _cached_flux_side_id = iside;

    const auto UL_3d = convert1DInputTo3D(UL_1d);
    const auto UR_3d = convert1DInputTo3D(UR_1d);

    const RealVectorValue nLR(nLR_dot_d.value(), 0, 0);
    RealVectorValue t1, t2;
    THM::computeOrthogonalDirections(nLR, t1, t2);

    calcFlux(UL_3d, UR_3d, nLR, t1, t2, _FL_3d, _FR_3d);

    transform3DFluxDirection(_FL_3d, nLR_dot_d);
    transform3DFluxDirection(_FR_3d, nLR_dot_d);

    _FL_1d = convert3DFluxTo1D(_FL_3d);
    _FR_1d = convert3DFluxTo1D(_FR_3d);
  }

  if (res_side_is_left)
    return _FL_1d;
  else
    return _FR_1d;
}

const std::vector<ADReal> &
NumericalFlux1D::getFlux3D(const unsigned int iside,
                           const dof_id_type ielem,
                           bool res_side_is_left,
                           const std::vector<ADReal> & UL_3d,
                           const std::vector<ADReal> & UR_3d,
                           const RealVectorValue & nLR,
                           const RealVectorValue & t1,
                           const RealVectorValue & t2) const
{
  if (_cached_flux_elem_id != ielem || _cached_flux_side_id != iside)
  {
    _cached_flux_elem_id = ielem;
    _cached_flux_side_id = iside;

    calcFlux(UL_3d, UR_3d, nLR, t1, t2, _FL_3d, _FR_3d);
  }

  if (res_side_is_left)
    return _FL_3d;
  else
    return _FR_3d;
}
