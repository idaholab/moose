//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrackMeshCut2DUserObject.h"

#include "XFEMFuncs.h"
#include "MooseError.h"
#include "libmesh/string_to_enum.h"
#include "MooseMesh.h"
#include "MooseEnum.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/mesh_tools.h"
#include "Function.h"

registerMooseObject("XFEMApp", CrackMeshCut2DUserObject);

InputParameters
CrackMeshCut2DUserObject::validParams()
{
  InputParameters params = GeometricCutUserObject::validParams();
  params.addRequiredParam<MeshFileName>(
      "mesh_file",
      "Mesh file for the XFEM geometric cut; currently only the Exodus type is supported");

  MooseEnum growthDirection("FUNCTION", "FUNCTION");
  params.addParam<MooseEnum>("growth_dir_method", growthDirection, "choose from FUNCTION");

  MooseEnum growthSpeed("FUNCTION", "FUNCTION");
  params.addParam<MooseEnum>("growth_speed_method", growthSpeed, "choose from FUNCTION");

  params.addRequiredParam<FunctionName>("function_x", "Growth function for x direction");
  params.addRequiredParam<FunctionName>("function_y", "Growth function for y direction");

  params.addRequiredParam<FunctionName>("function_v", "Growth speed function");

  params.addClassDescription("Creates a UserObject for a mesh cutter in 2D problems");
  return params;
}

CrackMeshCut2DUserObject::CrackMeshCut2DUserObject(const InputParameters & parameters)
  : GeometricCutUserObject(parameters),
    _mesh(_subproblem.mesh()),
    _growth_dir_method(getParam<MooseEnum>("growth_dir_method").getEnum<GrowthDirectionEnum>()),
    _growth_speed_method(getParam<MooseEnum>("growth_speed_method").getEnum<GrowthSpeedEnum>()),
    _func_x(&getFunction("function_x")),
    _func_y(&getFunction("function_y")),
    _func_v(&getFunction("function_v"))
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

void
CrackMeshCut2DUserObject::initialSetup()
{
  // setting _time_of_previous_call_to_UO to current time
  _time_of_previous_call_to_UO = _t;
}

void
CrackMeshCut2DUserObject::initialize()
{
  // following logic only calls crack growth function if time changed.
  // This deals with max_xfem_update > 1.  Error if the timestep drops below this
  if ((_t - _time_of_previous_call_to_UO) > libMesh::TOLERANCE)
  {
    findBoundaryNodes();
    findActiveBoundaryNodes();
    findActiveBoundaryDirection();
    growFront();
  }
  else if (_dt < libMesh::TOLERANCE && _t_step != 0)
    mooseError("timestep size must be greater than " + Moose::stringify(libMesh::TOLERANCE) +
               ", for crack to grow. dt=" + Moose::stringify(_dt));

  _time_of_previous_call_to_UO = _t;
}

bool
CrackMeshCut2DUserObject::cutElementByGeometry(const Elem * elem,
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
CrackMeshCut2DUserObject::cutElementByGeometry(const Elem * /*elem*/,
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
CrackMeshCut2DUserObject::cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_edges,
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
CrackMeshCut2DUserObject::cutFragmentByGeometry(std::vector<std::vector<Point>> & /*frag_faces*/,
                                                std::vector<Xfem::CutFace> & /*cut_faces*/) const
{
  mooseError("Invalid method for 2D mesh fragment cutting, must use "
             "vector of element edges for 2D mesh fragment cutting");
  return false;
}

void
CrackMeshCut2DUserObject::findBoundaryNodes()
{
  _boundary_node_ids.clear();
  std::unordered_set boundary_nodes = MeshTools::find_boundary_nodes(*_cutter_mesh);
  std::copy(boundary_nodes.begin(), boundary_nodes.end(), std::back_inserter(_boundary_node_ids));
}

void
CrackMeshCut2DUserObject::findActiveBoundaryNodes()
{
  _active_boundary_node_ids.clear();
  _inactive_boundary_node_ids.clear();

  std::unique_ptr<PointLocatorBase> pl = _mesh.getPointLocator();
  pl->enable_out_of_mesh_mode();

  // if the node is outside of the structural model, store its id to inactive_boundary_node_ids
  for (unsigned int j = 0; j < _boundary_node_ids.size(); ++j)
  {
    auto node_id = _boundary_node_ids[j];
    Node * this_node = _cutter_mesh->node_ptr(node_id);
    mooseAssert(this_node, "Node is NULL");
    Point & this_point = *this_node;

    const Elem * elem = (*pl)(this_point);
    if (elem == NULL)
      _inactive_boundary_node_ids.push_back(node_id);
    else
      _active_boundary_node_ids.push_back(node_id);
  }
}

void
CrackMeshCut2DUserObject::findActiveBoundaryDirection()
{
  _active_node_id_direction.clear();
  for (unsigned int i = 0; i < _active_boundary_node_ids.size(); ++i)
  {
    std::vector<Point> temp;
    Point dir;
    if (_growth_dir_method == GrowthDirectionEnum::FUNCTION)
    {
      // loop over active front points
      Node * this_node = _cutter_mesh->node_ptr(_active_boundary_node_ids[i]);
      mooseAssert(this_node, "Node is NULL");
      Point & this_point = *this_node;
      dir(0) = _func_x->value(_t, this_point);
      dir(1) = _func_y->value(_t, this_point);
      _active_node_id_direction.push_back(dir / dir.norm());
    }
    else
      mooseError("This growth_dir_method is not pre-defined!");
  }
}

void
CrackMeshCut2DUserObject::growFront()
{
  _active_boundary_node_front_ids.clear();
  for (unsigned int i = 0; i < _active_boundary_node_ids.size(); ++i)
  {
    Node * this_node = _cutter_mesh->node_ptr(_active_boundary_node_ids[i]);
    mooseAssert(this_node, "Node is NULL");
    Point & this_point = *this_node;
    Point dir = _active_node_id_direction[i];
    Point x;
    if (_growth_speed_method == GrowthSpeedEnum::FUNCTION)
      for (unsigned int k = 0; k < 3; ++k)
      {
        Real velo = _func_v->value(_t, Point(0, 0, 0));
        x(k) = this_point(k) + dir(k) * velo * _dt;
      }
    else
      mooseError("This growth_speed_method is not pre-defined!");

    // add node to front
    this_node = Node::build(x, _cutter_mesh->n_nodes()).release();
    _cutter_mesh->add_node(this_node);
    dof_id_type id = _cutter_mesh->n_nodes() - 1;
    _active_boundary_node_front_ids.push_back(id);

    // add element to front
    std::vector<dof_id_type> elem;
    elem.push_back(_active_boundary_node_ids[i]);
    elem.push_back(_active_boundary_node_front_ids.back());
    Elem * new_elem = Elem::build(EDGE2).release();
    for (unsigned int i = 0; i < new_elem->n_nodes(); ++i)
    {
      mooseAssert(_cutter_mesh->node_ptr(elem[i]) != nullptr, "Node is NULL");
      new_elem->set_node(i) = _cutter_mesh->node_ptr(elem[i]);
    }
    _cutter_mesh->add_elem(new_elem);
  }
  // rebuild connectivity so that find_boundary_nodes will return the new front node
  _cutter_mesh->prepare_for_use();
}

const std::vector<Point>
CrackMeshCut2DUserObject::getCrackFrontPoints(unsigned int /*number_crack_front_points*/) const
{
  mooseError("getCrackFrontPoints() is not implemented for CrackMeshCut2DUserObject.");
}

const std::vector<RealVectorValue>
CrackMeshCut2DUserObject::getCrackPlaneNormals(unsigned int /*number_crack_front_points*/) const
{
  mooseError("getCrackPlaneNormals() is not implemented for CrackMeshCut2DUserObject.");
}

MeshBase &
CrackMeshCut2DUserObject::getCutterMesh() const
{
  mooseAssert(_cutter_mesh, "CrackMeshCut2DUserObject::getCutterMesh _cutter_mesh is nullptr");
  return *_cutter_mesh;
}
