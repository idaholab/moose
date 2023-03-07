//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrackMeshCut2DUserObjectBase.h"

#include "XFEMFuncs.h"
#include "MooseError.h"
#include "MooseMesh.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/mesh_tools.h"

InputParameters
CrackMeshCut2DUserObjectBase::validParams()
{
  InputParameters params = GeometricCutUserObject::validParams();
  params.addRequiredParam<MeshFileName>(
      "mesh_file",
      "Mesh file for the XFEM geometric cut; currently only the Exodus type is supported");
  params.addClassDescription("Creates a UserObject base class for a mesh cutter in 2D problems");
  return params;
}

CrackMeshCut2DUserObjectBase::CrackMeshCut2DUserObjectBase(const InputParameters & parameters)
  : GeometricCutUserObject(parameters), _mesh(_subproblem.mesh())
{
  // only the Exodus type is currently supported
  MeshFileName cutterMeshFileName = getParam<MeshFileName>("mesh_file");
  _cutter_mesh = std::make_unique<ReplicatedMesh>(_communicator);
  _cutter_mesh->read(cutterMeshFileName);
  // test element type; only line elements are allowed
  for (const auto & cut_elem : _cutter_mesh->element_ptr_range())
  {
    if (cut_elem->n_nodes() != 2)
      mooseError("The input cut mesh should include EDGE2 elements only!");
    if (cut_elem->dim() != 1)
      mooseError("The input cut mesh should have 2D elements only!");
  }
}

bool
CrackMeshCut2DUserObjectBase::cutElementByGeometry(const Elem * elem,
                                                   std::vector<Xfem::CutEdge> & cut_edges,
                                                   std::vector<Xfem::CutNode> & cut_nodes) const
{
  // With the crack defined by a line, this method cuts a 2D elements by a line
  // Fixme lynn Copy and paste from InterfaceMeshCut2DUserObject::cutElementByGeometry
  mooseAssert(elem->dim() == 2, "Dimension of element to be cut must be 2");

  bool elem_cut = false;

  for (const auto & cut_elem : _cutter_mesh->element_ptr_range())
  {
    unsigned int n_sides = elem->n_sides();

    for (unsigned int i = 0; i < n_sides; ++i)
    {
      std::unique_ptr<const Elem> curr_side = elem->side_ptr(i);

      mooseAssert(curr_side->type() == EDGE2, "Element side type must be EDGE2.");

      const Node * node1 = curr_side->node_ptr(0);
      const Node * node2 = curr_side->node_ptr(1);
      Real seg_int_frac = 0.0;

      const std::pair<Point, Point> elem_endpoints(cut_elem->node_ref(0), cut_elem->node_ref(1));

      if (Xfem::intersectSegmentWithCutLine(*node1, *node2, elem_endpoints, 1.0, seg_int_frac))
      {
        if (seg_int_frac > Xfem::tol && seg_int_frac < 1.0 - Xfem::tol)
        {
          elem_cut = true;
          Xfem::CutEdge mycut;
          mycut._id1 = node1->id();
          mycut._id2 = node2->id();
          mycut._distance = seg_int_frac;
          mycut._host_side_id = i;
          cut_edges.push_back(mycut);
        }
        else if (seg_int_frac < Xfem::tol)
        {
          elem_cut = true;
          Xfem::CutNode mycut;
          mycut._id = node1->id();
          mycut._host_id = i;
          cut_nodes.push_back(mycut);
        }
      }
    }
  }
  return elem_cut;
}

bool
CrackMeshCut2DUserObjectBase::cutElementByGeometry(const Elem * /*elem*/,
                                                   std::vector<Xfem::CutFace> & /*cut_faces*/) const
{
  mooseError("Invalid method for 2D mesh cutting, must use "
             "vector of element edges for 2D mesh cutting");
  return false;
}

// fixme lynn
// sort of cut and paste from GeometricCut2DUserObject::cutFragmentByGeometry
// I get the exact same results whether this is called or not.  I don't konw what it does
// but if it is here, it definately returns true and pushback lots of cut_edges.
bool
CrackMeshCut2DUserObjectBase::cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_edges,
                                                    std::vector<Xfem::CutEdge> & cut_edges) const
{
  bool cut_frag = false;

  for (const auto & cut_elem : _cutter_mesh->element_ptr_range())
  {
    const std::pair<Point, Point> elem_endpoints(cut_elem->node_ref(0), cut_elem->node_ref(1));
    unsigned int n_sides = frag_edges.size();
    for (unsigned int i = 0; i < n_sides; ++i)
    {
      Real seg_int_frac = 0.0;
      if (Xfem::intersectSegmentWithCutLine(
              frag_edges[i][0], frag_edges[i][1], elem_endpoints, 1, seg_int_frac))
      {
        cut_frag = true;
        Xfem::CutEdge mycut;
        mycut._id1 = i;
        mycut._id2 = (i < (n_sides - 1) ? (i + 1) : 0);
        mycut._distance = seg_int_frac;
        mycut._host_side_id = i;
        cut_edges.push_back(mycut);
      }
    }
  }
  return cut_frag;
}

bool
CrackMeshCut2DUserObjectBase::cutFragmentByGeometry(
    std::vector<std::vector<Point>> & /*frag_faces*/,
    std::vector<Xfem::CutFace> & /*cut_faces*/) const
{
  mooseError("Invalid method for 2D mesh fragment cutting, must use "
             "vector of element edges for 2D mesh fragment cutting");
  return false;
}

MeshBase &
CrackMeshCut2DUserObjectBase::getCutterMesh() const
{
  mooseAssert(_cutter_mesh, "CrackMeshCut2DUserObjectBase::getCutterMesh _cutter_mesh is nullptr");
  return *_cutter_mesh;
}
