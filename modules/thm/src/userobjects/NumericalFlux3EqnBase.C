#include "NumericalFlux3EqnBase.h"

template <>
InputParameters
validParams<NumericalFlux3EqnBase>()
{
  InputParameters params = validParams<GeneralUserObject>();
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
                               const std::vector<Real> & UL,
                               const std::vector<Real> & UR,
                               const Real & nLR_dot_d) const
{
  if (_cached_flux_elem_id != ielem || _cached_flux_side_id != iside)
  {
    _cached_flux_elem_id = ielem;
    _cached_flux_side_id = iside;

    calcFlux(UL, UR, nLR_dot_d, _F);
  }
  return _F;
}

const DenseMatrix<Real> &
NumericalFlux3EqnBase::getJacobian(bool get_left_jacobian,
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

    calcJacobian(UL, UR, nLR_dot_d, _dF_dUL, _dF_dUR);
  }

  if (get_left_jacobian)
    return _dF_dUL;
  else
    return _dF_dUR;
}
