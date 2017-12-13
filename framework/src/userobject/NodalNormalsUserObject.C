//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalNormalsUserObject.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<NodalNormalsUserObject>()
{
  InputParameters params = validParams<NodalUserObject>();
  return params;
}

Threads::spin_mutex NodalNormalsUserObject::_add_contr;

NodalNormalsUserObject::NodalNormalsUserObject(const InputParameters & parameters)
  : NodalUserObject(parameters),
    _nodal_normals(declareRestartableData<std::map<dof_id_type, Point>>("nodal_normals"))
{
}

void
NodalNormalsUserObject::execute()
{
}

void
NodalNormalsUserObject::initialize()
{
}

void
NodalNormalsUserObject::finalize()
{
}

void
NodalNormalsUserObject::threadJoin(const UserObject & uo)
{
  const NodalNormalsUserObject & nnuo = dynamic_cast<const NodalNormalsUserObject &>(uo);

  for (auto & it : nnuo._nodal_normals)
    add(it.first, it.second);
}

Point
NodalNormalsUserObject::getNormal(dof_id_type node_id) const
{
  const auto & it = _nodal_normals.find(node_id);
  if (it != _nodal_normals.end())
    return it->second;
  else
    return Point(0, 0, 0);
}

void
NodalNormalsUserObject::add(const Node * node, RealGradient grad) const
{
  Threads::spin_mutex::scoped_lock lock(_add_contr);

  BoundaryInfo & boundary_info = _mesh.getMesh().get_boundary_info();
  std::vector<BoundaryID> node_boundary_ids;
  boundary_info.boundary_ids(node, node_boundary_ids);
  // Is the node part of the boundary I am restricted to?
  if (hasBoundary(node_boundary_ids, BoundaryRestrictable::ANY))
  {
    const dof_id_type & node_id = node->id();
    Point normal_contribution(grad);
    add(node_id, normal_contribution);
  }
}

void
NodalNormalsUserObject::add(dof_id_type node_id, const Point & contr) const
{
  const auto & it = _nodal_normals.find(node_id);
  if (it == _nodal_normals.end())
    _nodal_normals.insert(std::pair<dof_id_type, Point>(node_id, contr));
  else
    it->second += contr;
}

void
NodalNormalsUserObject::zeroNormals() const
{
  for (auto & it : _nodal_normals)
    it.second = Point(0, 0, 0);
}

void
NodalNormalsUserObject::computeNormals() const
{
  for (auto & it : _nodal_normals)
  {
    Point n = it.second;
    Real len = n.norm_sq();
    if (std::abs(len) >= 1e-13)
      it.second = n.unit();
  }
}

void
NodalNormalsUserObject::serialize(std::string & serialized_buffer) const
{
  std::ostringstream oss;

  unsigned int size = _nodal_normals.size();
  oss.write((char *)&size, sizeof(size));

  Point zero_normal(0, 0, 0);
  for (auto & it : _nodal_normals)
  {
    dof_id_type node_id = it.first;
    storeHelper(oss, node_id, const_cast<NodalNormalsUserObject *>(this));
    storeHelper(oss, it.second, const_cast<NodalNormalsUserObject *>(this));
  }

  // Populate the passed in string pointer with the string stream's buffer contents
  serialized_buffer.assign(oss.str());
}

void
NodalNormalsUserObject::deserialize(std::vector<std::string> & serialized_buffers) const
{
  // The input string stream used for deserialization
  std::istringstream iss;

  mooseAssert(serialized_buffers.size() == _app.n_processors(),
              "Unexpected size of serialized_buffers: " << serialized_buffers.size());

  for (auto rank = decltype(_app.n_processors())(0); rank < serialized_buffers.size(); ++rank)
  {
    if (rank == processor_id())
      continue;

    iss.str(serialized_buffers[rank]); // populate the stream with a new buffer
    iss.clear();                       // reset the string stream state

    // Load the communicated data into all of the other processors' slots

    unsigned int size = 0;
    iss.read((char *)&size, sizeof(size));

    for (unsigned int i = 0; i < size; i++)
    {
      dof_id_type node_id;
      loadHelper(iss, node_id, const_cast<NodalNormalsUserObject *>(this));

      Point normal;
      loadHelper(iss, normal, const_cast<NodalNormalsUserObject *>(this));

      // merge the data we received from other procs
      add(node_id, normal);
    }
  }
}

void
NodalNormalsUserObject::communicate() const
{
  std::vector<std::string> send_buffers(1);
  std::vector<std::string> recv_buffers;

  recv_buffers.reserve(_app.n_processors());
  serialize(send_buffers[0]);
  comm().allgather_packed_range((void *)(nullptr),
                                send_buffers.begin(),
                                send_buffers.end(),
                                std::back_inserter(recv_buffers));
  deserialize(recv_buffers);
}
