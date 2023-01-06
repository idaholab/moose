//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SelfShadowSideUserObject.h"
#include "RotationMatrix.h"

#include "libmesh/parallel_algebra.h"

registerMooseObject("HeatConductionApp", SelfShadowSideUserObject);

InputParameters
SelfShadowSideUserObject::validParams()
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

SelfShadowSideUserObject::SelfShadowSideUserObject(const InputParameters & parameters)
  : SideUserObject(parameters),
    _dim(_mesh.dimension()),
    _raw_direction(coupledPostprocessors("illumination_flux"))
{
  // we should check the coordinate system (i.e. permit only 0,0,+-1 for RZ)

  // check problem dimension
  if (_dim != 2 && _dim != 3)
    mooseError("SelfShadowSideUserObject works only for 2 and 3 dimensional problems.");

  // fetch raw direction
  for (const auto i : index_range(_raw_direction))
    _raw_direction[i] = &getPostprocessorValue("illumination_flux", i);
}

void
SelfShadowSideUserObject::initialize()
{
  // delete list of collected lines or triangles
  _lines.clear();
  _triangles.clear();

  // delete local qps
  _local_qps.clear();

  // update direction and rotation matrix
  RealVectorValue direction;
  for (const auto i : index_range(_raw_direction))
    direction(i) = *_raw_direction[i];
  _rotation =
      _dim == 2 ? RotationMatrix::rotVec2DToX(direction) : RotationMatrix::rotVecToZ(direction);
}

void
SelfShadowSideUserObject::execute()
{
  const SideIDType id(_current_elem->id(), _current_side);

  // add triangulated sides to list
  if (_dim == 2)
    addLines(id);
  else
    addTriangles(id);

  // save off rotated QP coordinates got local sides
  auto & qps = _local_qps[id];
  qps = _q_point;
  rotate(qps);
}

void
SelfShadowSideUserObject::threadJoin(const UserObject & y)
{
  const SelfShadowSideUserObject & uo = static_cast<const SelfShadowSideUserObject &>(y);

  // merge lists
  _lines.insert(_lines.begin(), uo._lines.begin(), uo._lines.end());
  _triangles.insert(_triangles.begin(), uo._triangles.begin(), uo._triangles.end());
}

int
SelfShadowSideUserObject::illumination(const SideIDType & id) const
{
  const auto it = _illumination_status.find(id);
  if (it == _illumination_status.end())
    mooseError("Illumination status was not calculated for current side.");
  return it->second;
}

void
SelfShadowSideUserObject::finalize()
{
  // rotate triangulations
  if (_dim == 2)
    for (auto & line : _lines)
      rotate(line.node);
  else
    for (auto & triangle : _triangles)
      rotate(triangle.node);

  // [compute local projected bounding box (in x or xy)]

  // communicate
  if (_dim == 2)
    _communicator.allgather(_lines, /*identical_buffer_sizes=*/false);
  else
    _communicator.allgather(_triangles, /*identical_buffer_sizes=*/false);

  // [check normal vector, if it points in the propagation direction (away from the source) then
  // the entire side cannot be illuminated] this would require normal vectors to be stored

  // otherwise we iterate over QPs and check if any other side is in the way of the radiation
  for (const auto & [id, qps] : _local_qps)
  {
    auto & illumination = _illumination_status[id];

    // start off assuming no illumination
    illumination = 0;

    // current bit in the illumination mask
    unsigned int bit = 1;

    // iterate over QPs
    for (const auto i : index_range(qps))
    {
      if (_dim == 2 && check2DIllumination(qps[i], id))
        illumination |= bit;
      if (_dim == 3 && check3DIllumination(qps[i], id))
        illumination |= bit;

      // shift to next bit
      bit <<= 1;
    }
  }
}

void
SelfShadowSideUserObject::addLines(const SideIDType & id)
{
  const auto & cse = *_current_side_elem;
  switch (cse.type())
  {
    case libMesh::EDGE2:
      _lines.push_back(LineSegment{{cse.node_ref(0), cse.node_ref(1)}, id});
      break;

    case libMesh::EDGE3:
      _lines.push_back(LineSegment{{cse.node_ref(0), cse.node_ref(2)}, id});
      _lines.push_back(LineSegment{{cse.node_ref(2), cse.node_ref(1)}, id});
      break;

    case libMesh::EDGE4:
      _lines.push_back(LineSegment{{cse.node_ref(0), cse.node_ref(2)}, id});
      _lines.push_back(LineSegment{{cse.node_ref(2), cse.node_ref(3)}, id});
      _lines.push_back(LineSegment{{cse.node_ref(3), cse.node_ref(1)}, id});
      break;

    default:
      mooseError("Unsupported EDGE type");
  }
}

void
SelfShadowSideUserObject::addTriangles(const SideIDType & id)
{
  const auto & cse = *_current_side_elem;
  switch (cse.type())
  {
    case libMesh::TRI3:
      _triangles.push_back(Triangle{{cse.node_ref(0), cse.node_ref(1), cse.node_ref(2)}, id});
      break;

    case libMesh::TRI6:
      _triangles.push_back(Triangle{{cse.node_ref(0), cse.node_ref(3), cse.node_ref(5)}, id});
      _triangles.push_back(Triangle{{cse.node_ref(3), cse.node_ref(1), cse.node_ref(4)}, id});
      _triangles.push_back(Triangle{{cse.node_ref(4), cse.node_ref(2), cse.node_ref(5)}, id});
      _triangles.push_back(Triangle{{cse.node_ref(3), cse.node_ref(4), cse.node_ref(5)}, id});
      break;

    case libMesh::QUAD4:
      _triangles.push_back(Triangle{{cse.node_ref(0), cse.node_ref(1), cse.node_ref(2)}, id});
      _triangles.push_back(Triangle{{cse.node_ref(2), cse.node_ref(3), cse.node_ref(0)}, id});
      break;

    case libMesh::QUAD8:
      _triangles.push_back(Triangle{{cse.node_ref(0), cse.node_ref(4), cse.node_ref(7)}, id});
      _triangles.push_back(Triangle{{cse.node_ref(4), cse.node_ref(1), cse.node_ref(5)}, id});
      _triangles.push_back(Triangle{{cse.node_ref(5), cse.node_ref(2), cse.node_ref(6)}, id});
      _triangles.push_back(Triangle{{cse.node_ref(6), cse.node_ref(3), cse.node_ref(7)}, id});
      _triangles.push_back(Triangle{{cse.node_ref(6), cse.node_ref(7), cse.node_ref(4)}, id});
      _triangles.push_back(Triangle{{cse.node_ref(4), cse.node_ref(5), cse.node_ref(6)}, id});
      break;

    case libMesh::QUAD9:
      _triangles.push_back(Triangle{{cse.node_ref(0), cse.node_ref(4), cse.node_ref(7)}, id});
      _triangles.push_back(Triangle{{cse.node_ref(4), cse.node_ref(1), cse.node_ref(5)}, id});
      _triangles.push_back(Triangle{{cse.node_ref(5), cse.node_ref(2), cse.node_ref(6)}, id});
      _triangles.push_back(Triangle{{cse.node_ref(6), cse.node_ref(3), cse.node_ref(7)}, id});
      _triangles.push_back(Triangle{{cse.node_ref(8), cse.node_ref(6), cse.node_ref(7)}, id});
      _triangles.push_back(Triangle{{cse.node_ref(8), cse.node_ref(5), cse.node_ref(6)}, id});
      _triangles.push_back(Triangle{{cse.node_ref(8), cse.node_ref(4), cse.node_ref(5)}, id});
      _triangles.push_back(Triangle{{cse.node_ref(8), cse.node_ref(7), cse.node_ref(4)}, id});
      break;

    default:
      mooseError("Unsupported FACE type");
  }
}

bool
SelfShadowSideUserObject::check2DIllumination(const Point & qp, const SideIDType & id)
{
  const auto x = qp(0);
  const auto y = qp(1);

  // loop over all line segments until one is found that provides shade
  for (const auto & line : _lines)
  {
    // make sure a side never shades itself
    if (line.id == id)
      continue;

    const int imax = line.node[0](1) > line.node[1](1) ? 0 : 1;
    const auto y1 = line.node[1 - imax](1);
    const auto y2 = line.node[imax](1);

    if (y >= y1 && y <= y2)
    {
      // segment is in line with the QP in radiation direction. Is it in front or behind?
      const auto x1 = line.node[1 - imax](0);
      const auto x2 = line.node[imax](0);

      // compute intersection location
      const auto xs = (x2 - x1) * (y - y1) / (y2 - y1) + x1;
      // tolerance is required to avoid an element to self shadow (might be problematic w. higher
      // order elements)
      if (x > xs)
        return false;
    }
  }

  return true;
}

bool
SelfShadowSideUserObject::check3DIllumination(const Point & qp, const SideIDType & id)
{
  const auto x = qp(0);
  const auto y = qp(1);
  const auto z = qp(2);

  // loop over all triangles until one is found that provides shade
  for (const auto & triangle : _triangles)
  {
    // make sure a side never shades itself
    if (triangle.id == id)
      continue;

    const auto x1 = triangle.node[0](0);
    const auto x2 = triangle.node[1](0);
    const auto x3 = triangle.node[2](0);

    const auto y1 = triangle.node[0](1);
    const auto y2 = triangle.node[1](1);
    const auto y3 = triangle.node[2](1);

    // compute barycentric coordinates
    const auto a = ((y2 - y3) * (x - x3) + (x3 - x2) * (y - y3)) /
                   ((y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3));
    const auto b = ((y3 - y1) * (x - x3) + (x1 - x3) * (y - y3)) /
                   ((y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3));
    const auto c = 1.0 - a - b;

    // are we inside of the projected triangle?
    if (0 <= a && a <= 1 && 0 <= b && b <= 1 && 0 <= c && c <= 1)
    {
      // Is the intersection it in front or behind the QP? (interpolate z using the barycentric
      // coordinates)
      const auto zs = a * triangle.node[0](2) + b * triangle.node[1](2) + c * triangle.node[2](2);
      if (z > zs)
        return false;
    }
  }

  return true;
}

template <typename T>
void
SelfShadowSideUserObject::rotate(T & points)
{
  for (const auto i : index_range(points))
    points[i] = _rotation * points[i];
}

namespace TIMPI
{

StandardType<SelfShadowSideUserObject::Triangle>::StandardType(
    const SelfShadowSideUserObject::Triangle * example)
{
  // We need an example for MPI_Address to use
  static const SelfShadowSideUserObject::Triangle p;
  if (!example)
    example = &p;

#ifdef LIBMESH_HAVE_MPI

  // Get the sub-data-types, and make sure they live long enough
  // to construct the derived type
  StandardType<std::array<Point, 3>> d1(&example->node);
  StandardType<SelfShadowSideUserObject::SideIDType> d2(&example->id);

  MPI_Datatype types[] = {(data_type)d1, (data_type)d2};
  int blocklengths[] = {1, 1};
  MPI_Aint displs[2], start;

  libmesh_call_mpi(
      MPI_Get_address(const_cast<SelfShadowSideUserObject::Triangle *>(example), &start));
  libmesh_call_mpi(MPI_Get_address(const_cast<std::array<Point, 3> *>(&example->node), &displs[0]));
  libmesh_call_mpi(MPI_Get_address(const_cast<SelfShadowSideUserObject::SideIDType *>(&example->id),
                                   &displs[1]));
  for (std::size_t i = 0; i < 2; ++i)
    displs[i] -= start;

  // create a prototype structure
  MPI_Datatype tmptype;
  libmesh_call_mpi(MPI_Type_create_struct(2, blocklengths, displs, types, &tmptype));
  libmesh_call_mpi(MPI_Type_commit(&tmptype));

  // resize the structure type to account for padding, if any
  libmesh_call_mpi(
      MPI_Type_create_resized(tmptype, 0, sizeof(SelfShadowSideUserObject::Triangle), &_datatype));
  libmesh_call_mpi(MPI_Type_free(&tmptype));

  this->commit();

#endif // LIBMESH_HAVE_MPI
}

StandardType<SelfShadowSideUserObject::Triangle>::StandardType(
    const StandardType<SelfShadowSideUserObject::Triangle> & t)
  : DataType(t._datatype)
{
#ifdef LIBMESH_HAVE_MPI
  libmesh_call_mpi(MPI_Type_dup(t._datatype, &_datatype));
#endif
}

StandardType<SelfShadowSideUserObject::LineSegment>::StandardType(
    const SelfShadowSideUserObject::LineSegment * example)
{
  // We need an example for MPI_Address to use
  static const SelfShadowSideUserObject::LineSegment p;
  if (!example)
    example = &p;

#ifdef LIBMESH_HAVE_MPI

  // Get the sub-data-types, and make sure they live long enough
  // to construct the derived type
  StandardType<std::array<Point, 2>> d1(&example->node);
  StandardType<SelfShadowSideUserObject::SideIDType> d2(&example->id);

  MPI_Datatype types[] = {(data_type)d1, (data_type)d2};
  int blocklengths[] = {1, 1};
  MPI_Aint displs[2], start;

  libmesh_call_mpi(
      MPI_Get_address(const_cast<SelfShadowSideUserObject::LineSegment *>(example), &start));
  libmesh_call_mpi(MPI_Get_address(const_cast<std::array<Point, 2> *>(&example->node), &displs[0]));
  libmesh_call_mpi(MPI_Get_address(const_cast<SelfShadowSideUserObject::SideIDType *>(&example->id),
                                   &displs[1]));
  for (std::size_t i = 0; i < 2; ++i)
    displs[i] -= start;

  // create a prototype structure
  MPI_Datatype tmptype;
  libmesh_call_mpi(MPI_Type_create_struct(2, blocklengths, displs, types, &tmptype));
  libmesh_call_mpi(MPI_Type_commit(&tmptype));

  // resize the structure type to account for padding, if any
  libmesh_call_mpi(MPI_Type_create_resized(
      tmptype, 0, sizeof(SelfShadowSideUserObject::LineSegment), &_datatype));
  libmesh_call_mpi(MPI_Type_free(&tmptype));

  this->commit();

#endif // LIBMESH_HAVE_MPI
}

StandardType<SelfShadowSideUserObject::LineSegment>::StandardType(
    const StandardType<SelfShadowSideUserObject::LineSegment> & t)
  : DataType(t._datatype)
{
#ifdef LIBMESH_HAVE_MPI
  libmesh_call_mpi(MPI_Type_dup(t._datatype, &_datatype));
#endif
}

}
