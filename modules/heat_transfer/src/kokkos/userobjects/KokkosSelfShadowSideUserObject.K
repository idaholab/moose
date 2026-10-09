//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosSelfShadowSideUserObject.h"

#include "RotationMatrix.h"

registerKokkosUserObject("HeatTransferApp", KokkosSelfShadowSideUserObject);

InputParameters
KokkosSelfShadowSideUserObject::validParams()
{
  InputParameters params = SideUserObject::validParams();
  params.addClassDescription("Compute the illumination status for a self shadowing sideset");
  params.addRequiredRangeCheckedParam<std::vector<PostprocessorName>>(
      "illumination_flux",
      "illumination_flux_size>0 && illumination_flux_size<=3",
      "Radiation direction vector. Each component of the vector can be a constant number or a "
      "postprocessor name (the latter enables time varying radiation directions - spatial "
      "variation is not supported)");
  return params;
}

KokkosSelfShadowSideUserObject::KokkosSelfShadowSideUserObject(const InputParameters & parameters)
  : SideUserObject(parameters),
    _dim(_subproblem.mesh().dimension()),
    _n_vertices(_dim == 2 ? 2 : 3),
    _max_sides(_subproblem.mesh().getMaxSidesPerElem()),
    _illumination(
        declareRestartableData<Moose::Kokkos::Array2D<unsigned int>>("illumination_status"))
{
  // Allocate the illumination state at construction so that consumers share its storage
  _illumination.create(_max_sides, _subproblem.mesh().nActiveLocalElem());
  _illumination = ~0u;

  // check problem dimension
  if (_dim != 2 && _dim != 3)
    mooseError("KokkosSelfShadowSideUserObject works only for 2 and 3 dimensional problems.");

  // fetch raw direction
  for (const auto i : make_range(coupledPostprocessors("illumination_flux")))
    _raw_direction.push_back(&getPostprocessorValue("illumination_flux", i));
}

void
KokkosSelfShadowSideUserObject::initialize()
{
  auto & mesh = _subproblem.mesh();

  // update direction and rotation matrix
  RealVectorValue direction;
  for (const auto i : index_range(_raw_direction))
    direction(i) = *_raw_direction[i];
  const RealTensorValue rotation =
      _dim == 2 ? RotationMatrix::rotVec2DToX(direction) : RotationMatrix::rotVecToZ(direction);
  _rotation = rotation;

  // collect the rotated primitives and side keys of the local sides
  std::vector<Real> points;
  std::vector<dof_id_type> keys;
  std::set<dof_id_type> added_sides;

  const auto & kokkos_mesh = kokkosMesh();

  mooseAssert(_illumination.n(1) == kokkos_mesh.getNumLocalElements(),
              "The number of local elements changed after construction");
  _side_keys.create(_max_sides, kokkos_mesh.getNumLocalElements());

  for (const auto boundary : boundaryIDs())
    for (const auto elem_id : mesh.getBoundaryActiveSemiLocalElemIds(boundary))
    {
      const auto * const elem = mesh.elemPtr(elem_id);
      if (elem->processor_id() != processor_id())
        continue;

      // an element can have several sides on the same boundary
      for (const auto side :
           mesh.getMesh().get_boundary_info().sides_with_boundary_id(elem, boundary))
      {
        const auto key = sideKey(*elem, side);

        // a side on multiple boundaries of this object is only added once
        if (!added_sides.insert(key).second)
          continue;

        _side_keys(side, kokkos_mesh.getContiguousElementID(elem)) = key;

        addPrimitives(*elem->build_side_ptr(side), key, points, keys);
      }
    }

  _side_keys.copyToDevice();

  // communicate
  _communicator.allgather(points, /*identical_buffer_sizes=*/false);
  _communicator.allgather(keys, /*identical_buffer_sizes=*/false);

  const auto num_primitives = keys.size();
  // points are stored by primitive, then vertex, then coordinate, which is the layout of
  // _primitives indexed by (coordinate, vertex, primitive)
  _primitives.create(Moose::dim, _n_vertices, num_primitives);
  for (const auto i : index_range(points))
    _primitives[i] = points[i];
  _primitives.copyToDevice();

  _primitive_ids = keys;
}

void
KokkosSelfShadowSideUserObject::compute()
{
  computeUserObject();

  // the illumination state is also read on the host for restart
  _illumination.copyToHost();
}

void
KokkosSelfShadowSideUserObject::addPrimitives(const Elem & side_elem,
                                              const dof_id_type key,
                                              std::vector<Real> & points,
                                              std::vector<dof_id_type> & keys) const
{
  const RealTensorValue rotation(_rotation(0, 0),
                                 _rotation(0, 1),
                                 _rotation(0, 2),
                                 _rotation(1, 0),
                                 _rotation(1, 1),
                                 _rotation(1, 2),
                                 _rotation(2, 0),
                                 _rotation(2, 1),
                                 _rotation(2, 2));

  // append one primitive in the rotated frame with the vertices given by local node indices
  auto add = [&](std::initializer_list<unsigned int> nodes)
  {
    for (const auto n : nodes)
    {
      const Point p = rotation * side_elem.point(n);
      for (const auto d : make_range(Moose::dim))
        points.push_back(p(d));
    }
    keys.push_back(key);
  };

  switch (side_elem.type())
  {
    case libMesh::EDGE2:
      add({0, 1});
      break;

    case libMesh::EDGE3:
      add({0, 2});
      add({2, 1});
      break;

    case libMesh::EDGE4:
      add({0, 2});
      add({2, 3});
      add({3, 1});
      break;

    case libMesh::TRI3:
      add({0, 1, 2});
      break;

    case libMesh::TRI6:
    case libMesh::TRI7:
      add({0, 3, 5});
      add({3, 1, 4});
      add({4, 2, 5});
      add({3, 4, 5});
      break;

    case libMesh::QUAD4:
      add({0, 1, 2});
      add({2, 3, 0});
      break;

    case libMesh::QUAD8:
      add({0, 4, 7});
      add({4, 1, 5});
      add({5, 2, 6});
      add({6, 3, 7});
      add({6, 7, 4});
      add({4, 5, 6});
      break;

    case libMesh::QUAD9:
      add({0, 4, 7});
      add({4, 1, 5});
      add({5, 2, 6});
      add({6, 3, 7});
      add({8, 6, 7});
      add({8, 5, 6});
      add({8, 4, 5});
      add({8, 7, 4});
      break;

    default:
      mooseError("Unsupported side element type");
  }
}
