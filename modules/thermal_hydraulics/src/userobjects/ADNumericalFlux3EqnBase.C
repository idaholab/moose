//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADNumericalFlux3EqnBase.h"
#include "THMIndicesVACE.h"
#include "THMUtils.h"

InputParameters
ADNumericalFlux3EqnBase::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription(
      "Base class for retrieving numerical fluxes in the the three-equation model");

  return params;
}

ADNumericalFlux3EqnBase::ADNumericalFlux3EqnBase(const InputParameters & parameters)
  : ThreadedGeneralUserObject(parameters),

    _cached_flux_elem_id(libMesh::invalid_uint),
    _cached_flux_side_id(libMesh::invalid_uint),

    _last_region_index(0)
{
}

void
ADNumericalFlux3EqnBase::initialize()
{
  _cached_flux_elem_id = libMesh::invalid_uint;
  _cached_flux_side_id = libMesh::invalid_uint;
}

void
ADNumericalFlux3EqnBase::execute()
{
}

void
ADNumericalFlux3EqnBase::finalize()
{
}

void
ADNumericalFlux3EqnBase::threadJoin(const UserObject &)
{
}

const std::vector<ADReal> &
ADNumericalFlux3EqnBase::getFlux(const unsigned int iside,
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

    std::vector<ADReal> UL_3d(THMVACE3D::N_FLUX_INPUTS);
    UL_3d[THMVACE3D::RHOA] = UL_1d[THMVACE1D::RHOA];
    UL_3d[THMVACE3D::RHOUA] = UL_1d[THMVACE1D::RHOUA];
    UL_3d[THMVACE3D::RHOVA] = 0;
    UL_3d[THMVACE3D::RHOWA] = 0;
    UL_3d[THMVACE3D::RHOEA] = UL_1d[THMVACE1D::RHOEA];
    UL_3d[THMVACE3D::AREA] = UL_1d[THMVACE1D::AREA];

    std::vector<ADReal> UR_3d(THMVACE3D::N_FLUX_INPUTS);
    UR_3d[THMVACE3D::RHOA] = UR_1d[THMVACE1D::RHOA];
    UR_3d[THMVACE3D::RHOUA] = UR_1d[THMVACE1D::RHOUA];
    UR_3d[THMVACE3D::RHOVA] = 0;
    UR_3d[THMVACE3D::RHOWA] = 0;
    UR_3d[THMVACE3D::RHOEA] = UR_1d[THMVACE1D::RHOEA];
    UR_3d[THMVACE3D::AREA] = UR_1d[THMVACE1D::AREA];

    const RealVectorValue nLR(nLR_dot_d.value(), 0, 0);
    RealVectorValue t1, t2;
    THM::computeOrthogonalDirections(nLR, t1, t2);

    calcFlux(UL_3d, UR_3d, nLR, t1, t2, _FL, _FR);

    _FL[THMVACE1D::MASS] *= nLR_dot_d;
    _FL[THMVACE1D::ENERGY] *= nLR_dot_d;

    _FR[THMVACE1D::MASS] *= nLR_dot_d;
    _FR[THMVACE1D::ENERGY] *= nLR_dot_d;
  }

  if (res_side_is_left)
    return _FL;
  else
    return _FR;
}
