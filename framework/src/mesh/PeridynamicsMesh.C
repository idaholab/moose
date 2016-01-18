/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "PeridynamicsMesh.h"
#include "NonlinearSystem.h"

// libMesh includes
#include "libmesh/getpot.h"
#include "libmesh/mesh_generation.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/periodic_boundaries.h"
#include "libmesh/periodic_boundary_base.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/boundary_info.h"
using namespace std;

/***********************************************************************************************/
/* Peridynamic Code */
/***********************************************************************************************/
struct node_structure2D
{	
  double X;
  double Y;
  int BondsNumPerNode;
  struct bond_structure
  {
    int NodesIndex;
  }bond[28]; // horizon = 3*Meshspacing 
};

struct node_structure3D
{
  double X;
  double Y;
  double Z;
  int BondsNumPerNode;
  struct bond_structure
  {
    int NodesIndex;
  }bond[122]; // horizon = 3*MeshSpacing
};

int CountNodeNum(double R,double dx,int nd) //applicable to disk shape only, 2, remove the nodes outside the circular domain
{
  int NodeNum = 0;
  double X = 0, Y = 0, dis = 0;

  for (int j = 0; j < nd; j++)
  {
    for (int i = 0; i < nd; i++)
    {
      X = -R  + static_cast<Real>(i)*dx;
      Y = -R  + static_cast<Real>(j)*dx;
      dis = sqrt(X*X + Y*Y);
      if (dis <= R + 0.001*dx)
        {
          NodeNum++;
        }
    }
  }
  return NodeNum;
}

void InitializeNode2D(struct node_structure2D *Node,int NodeNum)
{
  for(int i = 0; i < NodeNum; i++)
  {
    Node[i].X = 0.0;
    Node[i].Y = 0.0;
    Node[i].BondsNumPerNode = 0;
    for(int j = 0; j < 28; j++)
      {
        Node[i].bond[j].NodesIndex = 0;
      }
  }
}

void InitializeNode3D(struct node_structure3D *Node,int NodeNum)
{
  for(int i = 0; i < NodeNum; i++)
  {
    Node[i].X = 0.0;
    Node[i].Y = 0.0;
    Node[i].Z = 0.0;
    Node[i].BondsNumPerNode = 0;
    for(int j = 0; j < 122; j++)
    {
      Node[i].bond[j].NodesIndex = 0;
    }
  }
}

void SearchBond2D(struct node_structure2D *Node, int NodeNum, int Search_Range, double Horizon)
{
  int i, j, num;
  double dis;
  for(int node_id = 0; node_id < NodeNum; node_id++)
  {
    num = 0;
    i = (node_id - Search_Range >= 0) ? node_id - Search_Range : 0;
    j = (node_id + Search_Range <= NodeNum) ? node_id + Search_Range : NodeNum;
    for(int k = i; k < j; k++)
    {
      dis = sqrt(pow(Node[node_id].X - Node[k].X, 2) + pow(Node[node_id].Y - Node[k].Y, 2));
      if(dis <= 1.001 * Horizon && k != node_id)
      {
        Node[node_id].bond[num].NodesIndex = k;
        num++;
      }
    }
    Node[node_id].BondsNumPerNode = num;
  }
}

void SearchBond3D(struct node_structure3D *Node, int NodeNum, int Search_Range, double Horizon)
{
  int i, j, num;
  double dis;
  for(int node_id = 0; node_id < NodeNum; node_id++)
  {
    num = 0;
    i = (node_id - Search_Range >= 0) ? node_id - Search_Range : 0;
    j = (node_id + Search_Range <= NodeNum) ? node_id + Search_Range : NodeNum;
    for(int k = i; k < j; k++)
    {
      dis = sqrt(pow(Node[node_id].X - Node[k].X, 2) + pow(Node[node_id].Y - Node[k].Y, 2) + pow(Node[node_id].Z - Node[k].Z, 2));
      if(dis <= 1.001 * Horizon && k != node_id)
      {
        Node[node_id].bond[num].NodesIndex = k;
        num++;
      }
    }
    Node[node_id].BondsNumPerNode = num;
  }
}

/***********************************************************************************************/
template<>
InputParameters validParams<PeridynamicsMesh>()
{
  InputParameters params = validParams<MooseMesh>();

  MooseEnum dims("1=1 2 3");
  params.addRequiredParam<MooseEnum>("dim", dims, "Mesh dimension is required in Mesh Block"); // Make this parameter required
  params.addParam<int>("shape", 1, "1. Rectangular, 2. Disk"); // the shape of the domain cross section
  params.addRangeCheckedParam<int>("nx", 1, "nx>0", "Number of elements in the X/R directio");
  params.addParam<Real>("xmin", 0.0, "Lower X Coordinate of the generated mesh");
  params.addParam<Real>("ymin", 0.0, "Lower Y Coordinate of the generated mesh");
  params.addParam<Real>("zmin", 0.0, "Lower Z Coordinate of the generated mesh");
  params.addParam<Real>("xmax", 1.0, "Upper X Coordinate of the generated mesh");
  params.addParam<Real>("ymax", 1.0, "Upper Y Coordinate of the generated mesh");
  params.addParam<Real>("zmax", 1.0, "Upper Z Coordinate of the generated mesh");
  params.addParam<Real>("R", 1.0, "Radius of the circular domain if applicable");

  params.addParamNamesToGroup("dim", "Main");

  return params;
}

PeridynamicsMesh::PeridynamicsMesh(const InputParameters & parameters) :
  MooseMesh(parameters),
  _dim(getParam<MooseEnum>("dim")),
  _nx(getParam<int>("nx")),
  _shape(getParam<int>("shape")),
  _xmin(getParam<Real>("xmin")),
  _ymin(getParam<Real>("ymin")),
  _zmin(getParam<Real>("zmin")),
  _xmax(getParam<Real>("xmax")),
  _ymax(getParam<Real>("ymax")),
  _zmax(getParam<Real>("zmax")),
  _R(getParam<Real>("R"))
{
}

PeridynamicsMesh::~PeridynamicsMesh()
{
}

MooseMesh &
PeridynamicsMesh::clone() const
{
  return *(new PeridynamicsMesh(*this));
}

void
PeridynamicsMesh::buildMesh()
{
  ElemType elem_type = Utility::string_to_enum<ElemType>("EDGE2");
  UnstructuredMesh& mesh = dynamic_cast<UnstructuredMesh&>(getMesh());
  mesh.clear();
  mesh.set_mesh_dimension(1);
  BoundaryInfo& boundary_info = mesh.get_boundary_info();
  /***********************************************************************************************/
  /* Peridynamic Mesh */
  /***********************************************************************************************/
  double dis, X, Y, Z;
  unsigned int i, j, k, m, node_id;
  unsigned int NodeNum, NodeNumPerLayer, BondNum, nd, Search_Range;
  Point mypoint(0.0, 0.0, 0.0);

  if (_dim == 2)
  {
    struct node_structure2D *Node;
    if (_shape == 1) //rectangular domain
    {
      MeshSpacing = (_xmax - _xmin) / (_nx - 1);
      ny = static_cast<int>((_ymax - _ymin) / MeshSpacing) + 1;
      Horizon = MeshSpacing * 3.0; // use a large value for horizon
      // Create and Initialize Node Structure
      NodeNum = _nx * ny;
      Node = (struct node_structure2D*)malloc(NodeNum*sizeof(struct node_structure2D));
      InitializeNode2D(Node, NodeNum);
      mesh.reserve_nodes(NodeNum);
      // Define Nodal Coordinates
      for (j = 0, node_id = 0; j < ny; j++)
      {	
        for (i = 0; i < _nx; i++)
        {
          X = _xmin + static_cast<Real>(i)*MeshSpacing;
          Y = _ymin + static_cast<Real>(j)*MeshSpacing;	
          mypoint = Point(X, Y, 0.0);
          mesh.add_point (mypoint, node_id);
          Node[node_id].X = X;
          Node[node_id].Y = Y;
          node_id++;
        }
      }
      // Search Family Member
      Search_Range = 3 * _nx + 1;
      SearchBond2D(Node, NodeNum, Search_Range, Horizon);
      // Generate Mesh
      for(i = 0, j = 0; i < NodeNum; i++)
      {
        j += Node[i].BondsNumPerNode;
      }
      BondNum = j / 2;
      mesh.reserve_elem (BondNum);
      for (i = 0; i < NodeNum; i++)
      {
        for(j = 0; j < Node[i].BondsNumPerNode; j++)
        {	
          k = Node[i].bond[j].NodesIndex;
          if (k > i)
          {
            Elem* elem = mesh.add_elem (new Edge2);
            elem->set_node(0) = mesh.node_ptr(i);
            elem->set_node(1) = mesh.node_ptr(k);
            elem->subdomain_id() = 0;
          }		
        }
      }	
      // Define Boundary Nodes
      for (node_id = 0; node_id < NodeNum; node_id++)
      {
        if (Node[node_id].X < _xmin + MeshSpacing)
        {
          boundary_info.add_node(mesh.node_ptr(node_id),0);
        }
        if(Node[node_id].X > _xmax - MeshSpacing)
        {
          boundary_info.add_node(mesh.node_ptr(node_id),1);
        }
        if (Node[node_id].Y < _ymin + MeshSpacing)
        {
          boundary_info.add_node(mesh.node_ptr(node_id),2);
        }
        if (Node[node_id].Y > _ymax - MeshSpacing)
        {
          boundary_info.add_node(mesh.node_ptr(node_id),3);
        }
      }
      boundary_info.nodeset_name(0) = "Left";
      boundary_info.nodeset_name(1) = "Right";
      boundary_info.nodeset_name(2) = "Bottom";
      boundary_info.nodeset_name(3) = "Top";
    }
    else if (_shape == 2) //disk-like domain
    {
      MeshSpacing = 2 * _R / (2 *_nx - 2);
      Horizon = MeshSpacing * 3.0;
      // Create and Initialize Node Structure
      NodeNum = CountNodeNum(_R, MeshSpacing, 2 * _nx - 1);
      Node = (struct node_structure2D*)malloc(NodeNum*sizeof(struct node_structure2D));
      InitializeNode2D(Node, NodeNum);
      mesh.reserve_nodes(NodeNum);
      // Define Nodal Coordinates
      for (j = 0, node_id = 0; j < 2 * _nx - 1; j++)
      {
        for (i = 0; i < 2 * _nx - 1; i++)
        {
          X = -_R + static_cast<Real>(i) * MeshSpacing;
          Y = -_R + static_cast<Real>(j) * MeshSpacing;
          dis = sqrt(X * X + Y * Y);
          if (dis <= _R + 0.001*MeshSpacing)
          {
            mypoint = Point(X, Y, 0.0);
            mesh.add_point (mypoint, node_id);
            Node[node_id].X = X;
            Node[node_id].Y = Y;
            node_id++;
          }
        }
      }
      // Search Family Member
      Search_Range = 3 * 2 * _nx + 1;
      SearchBond2D(Node, NodeNum, Search_Range, Horizon);
      // Generate Mesh
      for(i = 0,j = 0; i < NodeNum; i++)
      {
        j += Node[i].BondsNumPerNode;
      }
      BondNum = j / 2;
      mesh.reserve_elem (BondNum);
      for (i = 0; i < NodeNum; i++)
      {
        for(j = 0; j < Node[i].BondsNumPerNode; j++)
        {
          k = Node[i].bond[j].NodesIndex;
          if (k > i)
          {
            Elem* elem = mesh.add_elem (new Edge2);
            elem->set_node(0) = mesh.node_ptr(i);
            elem->set_node(1) = mesh.node_ptr(k);
            elem->subdomain_id() = 0;
          }
        }
      }
      // Define Boundary Nodes
      for (node_id = 0; node_id < NodeNum; node_id++)
      {
        X = Node[node_id].X;
        Y = Node[node_id].Y;
        dis = sqrt(X * X + Y * Y);
        if (dis >= _R - MeshSpacing)
        {
          boundary_info.add_node(mesh.node_ptr(node_id),0);
        }
        if (dis < 0.001 * MeshSpacing)
        {
          boundary_info.add_node(mesh.node_ptr(node_id),1);
        }
        if(abs(Y) < 0.001 * MeshSpacing && dis > _R - MeshSpacing && X < 0.0)
        {
          boundary_info.add_node(mesh.node_ptr(node_id),2);
        }
        if (abs(Y) < 0.001 * MeshSpacing && dis > _R - MeshSpacing && X > 0.0)
        {
          boundary_info.add_node(mesh.node_ptr(node_id),3);
        }
        if(abs(X) < 0.001 * MeshSpacing && dis > _R - MeshSpacing && Y < 0.0)
        {
          boundary_info.add_node(mesh.node_ptr(node_id),4);
        }
        if(abs(X) < 0.001 * MeshSpacing && dis > _R - MeshSpacing && Y > 0.0)
        {
          boundary_info.add_node(mesh.node_ptr(node_id),5);
        }
      }
      boundary_info.nodeset_name(0) = "PeripheralCurve";
      boundary_info.nodeset_name(1) = "CenterPoint";
      boundary_info.nodeset_name(2) = "LeftPoint";
      boundary_info.nodeset_name(3) = "RightPoint";
      boundary_info.nodeset_name(4) = "BottomPoint";
      boundary_info.nodeset_name(5) = "TopPoint";
    }
  }
  else if(_dim == 3)
  {
    struct node_structure3D *Node;
    if (_shape == 1) //rectangular domain
    {
      MeshSpacing = (_xmax - _xmin) / (_nx - 1);
      ny = static_cast<int>((_ymax - _ymin) / MeshSpacing) + 1;
      nz = static_cast<int>((_zmax - _zmin) / MeshSpacing) + 1;
      Horizon = MeshSpacing * 3.0; // use the largest horizon
      // Create and Initialize Node Structure
      NodeNum = _nx * ny * nz;
      Node = (struct node_structure3D*)malloc(NodeNum*sizeof(struct node_structure3D));
      InitializeNode3D(Node, NodeNum);
      mesh.reserve_nodes(NodeNum);
      // Define Nodal Coordinates
      for(k = 0, node_id = 0; k < nz; k++)
      {
        for (j = 0; j < ny; j++)
        {
          for (i = 0; i < _nx; i++)
          {
            X = _xmin + static_cast<Real>(i) * MeshSpacing;
            Y = _ymin + static_cast<Real>(j) * MeshSpacing;
            Z = _zmin + static_cast<Real>(k) * MeshSpacing;
            mypoint = Point(X, Y, Z);
            mesh.add_point (mypoint, node_id);
            Node[node_id].X = X;
            Node[node_id].Y = Y;
            Node[node_id].Z = Z;
            node_id++;
          }
        }
      }
      // Search Family Member
      NodeNumPerLayer = _nx * ny;
      Search_Range = 3 * NodeNumPerLayer + ny;
      SearchBond3D(Node, NodeNum, Search_Range, Horizon);
      // Generate Mesh
      for(i = 0, j = 0; i < NodeNum; i++)
      {
        j += Node[i].BondsNumPerNode;
      }
      BondNum = j / 2;
      mesh.reserve_elem (BondNum);
      for (i = 0; i < NodeNum; i++)
        {
          for(j = 0; j < Node[i].BondsNumPerNode; j++)
          {
            k = Node[i].bond[j].NodesIndex;
            if (k > i)
            {
              Elem* elem = mesh.add_elem (new Edge2);
              elem->set_node(0) = mesh.node_ptr(i);
              elem->set_node(1) = mesh.node_ptr(k);
              elem->subdomain_id() = 0;
            }
          }
        }
        // Define Boundary Nodes
        for (node_id = 0; node_id < NodeNum; node_id++)
        {
          if (Node[node_id].Y < _ymin + MeshSpacing)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),0);
          }
          if(Node[node_id].Y > _ymax - MeshSpacing)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),1);
          }
          if (Node[node_id].Z < _zmin + MeshSpacing)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),2);
          }
          if (Node[node_id].Z > _zmax - MeshSpacing)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),3);
          }
          if (Node[node_id].X < _xmin + MeshSpacing)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),4);
          }
          if (Node[node_id].X > _xmax - MeshSpacing)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),5);
          }
        }
        boundary_info.nodeset_name(0) = "Left";
        boundary_info.nodeset_name(1) = "Right";
        boundary_info.nodeset_name(2) = "Bottom";
        boundary_info.nodeset_name(3) = "Top";
        boundary_info.nodeset_name(4) = "Back";
        boundary_info.nodeset_name(5) = "Front";
      }
      else if (_shape == 2) // cylinder shape
      {
        MeshSpacing = 2.0 * _R / (2 * _nx - 2);
        nz = static_cast<int>((_zmax - _zmin) / MeshSpacing) + 1;
        Horizon = MeshSpacing * 3.0;
        // Create and Initialize Node Structure
        NodeNumPerLayer = CountNodeNum(_R, MeshSpacing, 2 * _nx - 1);
        NodeNum = NodeNumPerLayer * nz;
        Node = (struct node_structure3D*)malloc(NodeNum*sizeof(struct node_structure3D));
        InitializeNode3D(Node, NodeNum);
        mesh.reserve_nodes(NodeNum);
        // Define Nodal Coordinates
        for (k = 0, node_id = 0; k < nz; k++)
        {
          for (j = 0; j < 2 * _nx - 1; j++)
          {
            for (i = 0; i < 2 * _nx - 1; i++)
            {
              X = -_R + static_cast<Real>(i) * MeshSpacing;
              Y = -_R + static_cast<Real>(j) * MeshSpacing;
              Z = _zmin + static_cast<Real>(k) * MeshSpacing;
              dis = sqrt(X * X + Y * Y);
              if (dis <= _R + 0.001 * MeshSpacing)
              {
                mypoint = Point(X, Y, Z);
                mesh.add_point (mypoint, node_id);
                Node[node_id].X = X;
                Node[node_id].Y = Y;
                Node[node_id].Z = Z;
                node_id++;
              }
            }
          }
        }
        // Search Family Member
        Search_Range = 3 * NodeNumPerLayer + _nx;
        SearchBond3D(Node, NodeNum, Search_Range, Horizon);
        // Generate Mesh
        for(i = 0, j = 0; i < NodeNum; i++)
        {
          j += Node[i].BondsNumPerNode;
        }
        BondNum = j / 2;
        mesh.reserve_elem (BondNum);
        for (i = 0; i < NodeNum; i++)
        {
          for(j = 0; j < Node[i].BondsNumPerNode; j++)
          {
            k = Node[i].bond[j].NodesIndex;
            if (k > i)
            {
              Elem* elem = mesh.add_elem (new Edge2);
              elem->set_node(0) = mesh.node_ptr(i);
              elem->set_node(1) = mesh.node_ptr(k);
              elem->subdomain_id() = 0;
            }
          }
        }
        // Define Boundary Nodes
        for (node_id = 0; node_id < NodeNum; node_id++)
        {
          X = Node[node_id].X;
          Y = Node[node_id].Y;
          Z = Node[node_id].Z;
          dis = sqrt(X * X + Y * Y);
          if (dis >= _R - MeshSpacing)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),0);
          }
          if (dis < 0.001 * MeshSpacing)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),1);
          }
          if (abs(Y) < 0.001 * MeshSpacing && dis > _R - MeshSpacing && X < 0.0)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),2);
          }
          if (abs(Y) < 0.001 * MeshSpacing && dis > _R - MeshSpacing && X > 0.0)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),3);
          }
          if (abs(X) < 0.001 * MeshSpacing && dis > _R - MeshSpacing && Y < 0.0)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),4);
          }
          if (abs(X) < 0.001 * MeshSpacing && dis > _R - MeshSpacing && Y > 0.0)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),5);
          }
          if (Z < _zmin + MeshSpacing)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),6);
          }
          if (Z > _zmax - MeshSpacing)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),7);
          }
        }
        boundary_info.nodeset_name(0) = "PeripheralSurface";
        boundary_info.nodeset_name(1) = "CenterLine";
        boundary_info.nodeset_name(2) = "LeftLine";
        boundary_info.nodeset_name(3) = "RightLine";
        boundary_info.nodeset_name(4) = "BackLine";
        boundary_info.nodeset_name(5) = "FrontLine";
        boundary_info.nodeset_name(6) = "Bottom";
        boundary_info.nodeset_name(7) = "Top";
      }
    }
    std::cout << "NodeNum = " << NodeNum << std::endl;
    std::cout << "BondNum = " << BondNum << std::endl;

    // Prepare for use 
    mesh.prepare_for_use (/*skip_renumber =*/ false);

/***********************************************************************************************/
}
