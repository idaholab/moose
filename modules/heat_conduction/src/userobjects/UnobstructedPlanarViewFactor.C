//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UnobstructedPlanarViewFactor.h"
#include "Assembly.h"

#include "libmesh/quadrature.h"
#include "libmesh/fe_base.h"
#include "libmesh/mesh_generation.h"
#include "libmesh/mesh.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/point_locator_base.h"
#include "libmesh/elem.h"

registerMooseObject("HeatConductionApp", UnobstructedPlanarViewFactor);

template <>
InputParameters
validParams<UnobstructedPlanarViewFactor>()
{
  InputParameters params = validParams<ViewFactorBase>();
  params.addClassDescription(
      "Computes the view factors for planar faces in unubstructed radiative heat transfer.");
  return params;
}

UnobstructedPlanarViewFactor::UnobstructedPlanarViewFactor(const InputParameters & parameters)
  : ViewFactorBase(parameters),
    _boundary_info(nullptr),
    _current_remote_side(nullptr),
    _current_remote_fe(nullptr),
    _current_remote_JxW(nullptr),
    _current_remote_xyz(nullptr),
    _current_remote_normals(nullptr)
{
  _mesh.errorIfDistributedMesh("UnobstructedPlanarViewFactor");
}

void
UnobstructedPlanarViewFactor::execute()
{
  auto current_boundary_name = _mesh.getBoundaryName(_current_boundary_id);
  if (_side_name_index.find(current_boundary_name) == _side_name_index.end())
    mooseError("Current boundary name: ",
               current_boundary_name,
               " with id ",
               _current_boundary_id,
               " not in boundary parameter.");
  unsigned int index = _side_name_index.find(current_boundary_name)->second;

  _areas[index] += _current_side_volume;

  for (auto & side : _side_list)
  {
    auto remote_boundary_name = _mesh.getBoundaryName(std::get<2>(side));
    if (_side_name_index.find(remote_boundary_name) != _side_name_index.end() &&
        std::get<2>(side) != _current_boundary_id)
    {
      // this is the remote side index
      unsigned int remote_index = _side_name_index.find(remote_boundary_name)->second;
      Real & vf = _view_factors[index][remote_index];

      // compute some important quantities on the face
      reinitFace(std::get<0>(side), std::get<1>(side));

      // loop over pairs of the qps on the current side
      for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
      {
        // loop over the qps on the remote element
        for (unsigned int r_qp = 0; r_qp < _current_remote_JxW->size(); ++r_qp)
        {
          Point r2r = (_q_point[qp] - (*_current_remote_xyz)[r_qp]);
          Real distance = r2r.norm();
          Real cos1 = r2r * _normals[qp] / distance;
          Real cos2 = r2r * (*_current_remote_normals)[r_qp] / distance;

          vf += _JxW[qp] * _coord[qp] * (*_current_remote_JxW)[r_qp] * _current_remote_coord[r_qp] *
                std::abs(cos1) * std::abs(cos2) / distance / distance;
        }
      }
    }
  }
}

void
UnobstructedPlanarViewFactor::initialize()
{
  // get boundary info from the mesh
  _boundary_info = &_mesh.getMesh().get_boundary_info();

  // get a list of all sides
  _side_list = _boundary_info->build_active_side_list();

  // set view_factors to zero
  for (unsigned int j = 0; j < _n_sides; ++j)
  {
    _areas[j] = 0;
    for (auto & vf : _view_factors[j])
      vf = 0;
  }
}

void
UnobstructedPlanarViewFactor::finalizeViewFactor()
{
  gatherSum(_areas);

  // divide view_factor Fij by Ai and pi
  for (unsigned int i = 0; i < _n_sides; ++i)
    for (auto & vf : _view_factors[i])
      vf /= (_areas[i] * libMesh::pi);
}

void
UnobstructedPlanarViewFactor::threadJoinViewFactor(const UserObject & y)
{
  const UnobstructedPlanarViewFactor & pps = static_cast<const UnobstructedPlanarViewFactor &>(y);
  for (unsigned int i = 0; i < _n_sides; ++i)
    _areas[i] += pps._areas[i];
}

void
UnobstructedPlanarViewFactor::reinitFace(dof_id_type elem_id, unsigned int side)
{
  const Elem * current_remote_elem = _mesh.getMesh().elem_ptr(elem_id);
  _current_remote_side = current_remote_elem->build_side_ptr(side);
  _current_remote_side_volume = _current_remote_side->volume();

  Order order = current_remote_elem->default_order();
  unsigned int dim = _mesh.getMesh().mesh_dimension();
  _current_remote_fe = FEBase::build(dim, FEType(order));
  QGauss qface(dim - 1, FEType(order).default_quadrature_order());
  _current_remote_fe->attach_quadrature_rule(&qface);

  _current_remote_JxW = &_current_remote_fe->get_JxW();
  _current_remote_normals = &_current_remote_fe->get_normals();
  _current_remote_xyz = &_current_remote_fe->get_xyz();

  _current_remote_fe->reinit(current_remote_elem, side);

  // set _coord
  unsigned int n_points = _current_remote_xyz->size();
  unsigned int rz_radial_coord = _subproblem.getAxisymmetricRadialCoord();
  _current_remote_coord.resize(n_points);
  switch (_assembly.coordSystem())
  {
    case Moose::COORD_XYZ:
      for (unsigned int qp = 0; qp < n_points; qp++)
        _current_remote_coord[qp] = 1.;
      break;

    case Moose::COORD_RZ:
      for (unsigned int qp = 0; qp < n_points; qp++)
        _current_remote_coord[qp] = 2 * M_PI * (*_current_remote_xyz)[qp](rz_radial_coord);
      break;

    case Moose::COORD_RSPHERICAL:
      for (unsigned int qp = 0; qp < n_points; qp++)
        _current_remote_coord[qp] =
            4 * M_PI * (*_current_remote_xyz)[qp](0) * (*_current_remote_xyz)[qp](0);
      break;

    default:
      mooseError("Unknown coordinate system");
      break;
  }
}
