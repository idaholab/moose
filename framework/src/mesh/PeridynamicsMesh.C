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
#include "libmesh/edge_edge2.h"
#include "libmesh/boundary_info.h"

struct node_structure2D
{	
  double X;
  double Y;
  int bonds_per_node;
  struct bond_structure
  {
    int node_index;
  }bond[37]; // horizon = 3*Meshspacing, 28 for square shape, 37 for disk shape 
};

struct node_structure3D
{
  double X;
  double Y;
  double Z;
  int bonds_per_node;
  struct bond_structure
  {
    int node_index;
  }bond[122]; // horizon = 3*MeshSpacing, 122 for cube shape, 118 for cylinder shape
};

void InitializeNode2D(struct node_structure2D *node,int node_num)
{
  for(int i = 0; i < node_num; i++)
  {
    node[i].X = 0.0;
    node[i].Y = 0.0;
    node[i].bonds_per_node = 0;
    for(int j = 0; j < 37; j++)
      {
        node[i].bond[j].node_index = 0;
      }
  }
}

void InitializeNode3D(struct node_structure3D *node,int node_num)
{
  for(int i = 0; i < node_num; i++)
  {
    node[i].X = 0.0;
    node[i].Y = 0.0;
    node[i].Z = 0.0;
    node[i].bonds_per_node = 0;
    for(int j = 0; j < 122; j++)
    {
      node[i].bond[j].node_index = 0;
    }
  }
}

void SearchBond2D(struct node_structure2D *node, int node_num, int search_range, double horizon)
{
  int i, j, num;
  double dis;
  for(int node_id = 0; node_id < node_num; node_id++)
  {
    num = 0;
    i = (node_id - search_range >= 0) ? node_id - search_range : 0;
    j = (node_id + search_range <= node_num) ? node_id + search_range : node_num;
    for(int k = i; k < j; k++)
    {
      dis = std::sqrt(std::pow(node[node_id].X - node[k].X, 2) + std::pow(node[node_id].Y - node[k].Y, 2));
      if(dis <= horizon && k != node_id)
      {
        node[node_id].bond[num].node_index = k;
        num++;
      }
    }
    node[node_id].bonds_per_node = num;
  }
}

void SearchBond3D(struct node_structure3D *node, int node_num, int search_range, double horizon)
{
  int i, j, num;
  double dis;
  for(int node_id = 0; node_id < node_num; node_id++)
  {
    num = 0;
    i = (node_id - search_range >= 0) ? node_id - search_range : 0;
    j = (node_id + search_range <= node_num) ? node_id + search_range : node_num;
    for(int k = i; k < j; k++)
    {
      dis = std::sqrt(std::pow(node[node_id].X - node[k].X, 2) + std::pow(node[node_id].Y - node[k].Y, 2) + std::pow(node[node_id].Z - node[k].Z, 2));
      if(dis <= horizon && k != node_id)
      {
        node[node_id].bond[num].node_index = k;
        num++;
      }
    }
    node[node_id].bonds_per_node = num;
  }
}

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
  UnstructuredMesh& mesh = dynamic_cast<UnstructuredMesh&>(getMesh());
  mesh.clear();
  mesh.set_mesh_dimension(1);
  BoundaryInfo& boundary_info = mesh.get_boundary_info();
  
  double dis, X, Y, Z;
  int i, j, k, n, node_id = 0;
  int node_num = 0, bond_num = 0, node_per_layer, search_range;

  if (_dim == 2)
  {
    struct node_structure2D *node;
    if (_shape == 1) //rectangular domain
    {
      mesh_spacing = (_xmax - _xmin) / (_nx - 1);
      ny = static_cast<int>((_ymax - _ymin) / mesh_spacing) + 1;
      horizon = mesh_spacing * 3.0; // use a large value for horizon
      // Create and Initialize Node Structure
      node_num = _nx * ny;
      node = (struct node_structure2D*)malloc(node_num * sizeof(struct node_structure2D));
      InitializeNode2D(node, node_num);
      mesh.reserve_nodes(node_num);
      // Define Nodal Coordinates
      for (j = 0; j < ny; j++)
      {	
        for (i = 0; i < _nx; i++)
        {
          X = _xmin + i * mesh_spacing;
          Y = _ymin + j * mesh_spacing;	
          mesh.add_point (Point(X, Y, 0.0), node_id);
          node[node_id].X = X;
          node[node_id].Y = Y;
          node_id++;
        }
      }
      // Search Family Member
      search_range = 3 * _nx + 1;
      SearchBond2D(node, node_num, search_range, horizon);
      // Generate Mesh
      for(i = 0, j = 0; i < node_num; i++)
      {
        j += node[i].bonds_per_node;
      }
      bond_num = j / 2;
      mesh.reserve_elem (bond_num);
      for (i = 0; i < node_num; i++)
      {
        for(j = 0; j < node[i].bonds_per_node; j++)
        {	
          k = node[i].bond[j].node_index;
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
      for (node_id = 0; node_id < node_num; node_id++)
      {
        if (node[node_id].X < _xmin + mesh_spacing)
        {
          boundary_info.add_node(mesh.node_ptr(node_id),0);
        }
        if(node[node_id].X > _xmax - mesh_spacing)
        {
          boundary_info.add_node(mesh.node_ptr(node_id),1);
        }
        if (node[node_id].Y < _ymin + mesh_spacing)
        {
          boundary_info.add_node(mesh.node_ptr(node_id),2);
        }
        if (node[node_id].Y > _ymax - mesh_spacing)
        {
          boundary_info.add_node(mesh.node_ptr(node_id),3);
        }
        if (std::abs((_xmax - _xmin) * (_ymin - node[node_id].Y) - (_xmin - node[node_id].X) * (_ymax - _ymin)) / std::sqrt(std::pow(_xmax - _xmin,2) + std::pow(_ymax - _ymin,2)) < 0.1*mesh_spacing)
        {
          boundary_info.add_node(mesh.node_ptr(node_id),4);
        }
      }
      boundary_info.nodeset_name(0) = "Left";
      boundary_info.nodeset_name(1) = "Right";
      boundary_info.nodeset_name(2) = "Bottom";
      boundary_info.nodeset_name(3) = "Top";
      boundary_info.nodeset_name(4) = "Diag";
    }
    else if (_shape == 2) //disk-like domain
    {
      mesh_spacing = _R / (_nx - 1);
      horizon = mesh_spacing * 3.0;
      // Create and Initialize Node Structure
      node_num++;
      for (i = 1; i < _nx; i++)
      {
        j = std::floor(3.14159265359 / std::asin(1.0 / (2.0 * i)) + 0.5);
        node_num += j;
      }
      node = (struct node_structure2D*)malloc(node_num * sizeof(struct node_structure2D));
      InitializeNode2D(node, node_num);
      mesh.reserve_nodes(node_num);
      // Define Nodal Coordinates
      mesh.add_point (Point(0.0, 0.0, 0.0), node_id);
      node[node_id].X = 0.0;
      node[node_id].Y = 0.0;
      node_id++;
      for (i = 1; i < _nx; i++)
      {
        k = std::floor(3.14159265359 / std::asin(1.0 / (2.0 * i)) + 0.5);
        for (j = 0; j < k; j++)
        {
          X = i * mesh_spacing * std::cos(j * 2.0 * 3.14159265359 / k);
          Y = i * mesh_spacing * std::sin(j * 2.0 * 3.14159265359 / k);
          mesh.add_point (Point(X, Y, 0.0), node_id);
          node[node_id].X = X;
          node[node_id].Y = Y;
          node_id++;
        }
      }
      // Search Family Member
      search_range = node_num;
      SearchBond2D(node, node_num, search_range, horizon);
      // Generate Mesh
      for(i = 0,j = 0; i < node_num; i++)
      {
        j += node[i].bonds_per_node;
      }
      bond_num = j / 2;
      mesh.reserve_elem (bond_num);
      for (i = 0; i < node_num; i++)
      {
        for(j = 0; j < node[i].bonds_per_node; j++)
        {
          k = node[i].bond[j].node_index;
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
      for (node_id = 0; node_id < node_num; node_id++)
      {
        X = node[node_id].X;
        Y = node[node_id].Y;
        dis = std::sqrt(X * X + Y * Y);
        if (dis > _R - mesh_spacing)
        {
          boundary_info.add_node(mesh.node_ptr(node_id),0);
        }
        if (dis < 0.001 * mesh_spacing)
        {
          boundary_info.add_node(mesh.node_ptr(node_id),1);
        }
        if(std::abs(Y) < 0.001 * mesh_spacing && dis > _R - mesh_spacing && X < 0.0)
        {
          boundary_info.add_node(mesh.node_ptr(node_id),2);
        }
        if (std::abs(Y) < 0.001 * mesh_spacing && dis > _R - mesh_spacing && X > 0.0)
        {
          boundary_info.add_node(mesh.node_ptr(node_id),3);
        }
        if(std::abs(X) < 0.001 * mesh_spacing && dis > _R - mesh_spacing && Y < 0.0)
        {
          boundary_info.add_node(mesh.node_ptr(node_id),4);
        }
        if(std::abs(X) < 0.001 * mesh_spacing && dis > _R - mesh_spacing && Y > 0.0)
        {
          boundary_info.add_node(mesh.node_ptr(node_id),5);
        }
      }
      boundary_info.nodeset_name(0) = "Periphery";
      boundary_info.nodeset_name(1) = "CenterPoint";
      boundary_info.nodeset_name(2) = "LeftPoint";
      boundary_info.nodeset_name(3) = "RightPoint";
      boundary_info.nodeset_name(4) = "BottomPoint";
      boundary_info.nodeset_name(5) = "TopPoint";
    }
  }
  else if(_dim == 3)
  {
    struct node_structure3D *node;
    if (_shape == 1) //rectangular domain
    {
      mesh_spacing = (_xmax - _xmin) / (_nx - 1);
      ny = static_cast<int>((_ymax - _ymin) / mesh_spacing) + 1;
      nz = static_cast<int>((_zmax - _zmin) / mesh_spacing) + 1;
      horizon = mesh_spacing * 3.0; // use the largest horizon
      // Create and Initialize Node Structure
      node_num = _nx * ny * nz;
      node = (struct node_structure3D*)malloc(node_num*sizeof(struct node_structure3D));
      InitializeNode3D(node, node_num);
      mesh.reserve_nodes(node_num);
      // Define Nodal Coordinates
      for(k = 0; k < nz; k++)
      {
        for (j = 0; j < ny; j++)
        {
          for (i = 0; i < _nx; i++)
          {
            X = _xmin + i * mesh_spacing;
            Y = _ymin + j * mesh_spacing;
            Z = _zmin + k * mesh_spacing;
            mesh.add_point (Point(X, Y, Z), node_id);
            node[node_id].X = X;
            node[node_id].Y = Y;
            node[node_id].Z = Z;
            node_id++;
          }
        }
      }
      // Search Family Member
      node_per_layer = _nx * ny;
      search_range = 3 * node_per_layer + ny;
      SearchBond3D(node, node_num, search_range, horizon);
      // Generate Mesh
      for(i = 0, j = 0; i < node_num; i++)
      {
        j += node[i].bonds_per_node;
      }
      bond_num = j / 2;
      mesh.reserve_elem (bond_num);
      for (i = 0; i < node_num; i++)
        {
          for(j = 0; j < node[i].bonds_per_node; j++)
          {
            k = node[i].bond[j].node_index;
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
        for (node_id = 0; node_id < node_num; node_id++)
        {
          if (node[node_id].Y < _ymin + mesh_spacing)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),0);
          }
          if(node[node_id].Y > _ymax - mesh_spacing)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),1);
          }
          if (node[node_id].Z < _zmin + mesh_spacing)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),2);
          }
          if (node[node_id].Z > _zmax - mesh_spacing)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),3);
          }
          if (node[node_id].X < _xmin + mesh_spacing)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),4);
          }
          if (node[node_id].X > _xmax - mesh_spacing)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),5);
          }
          if (std::sqrt(((std::pow(_xmin - node[node_id].X,2) + std::pow(_ymin - node[node_id].Y,2) + std::pow(_zmin - node[node_id].Z,2)) * (std::pow(_xmax - _xmin,2) + std::pow(_ymax - _ymin,2) + std::pow(_zmax - _zmin,2)) - std::pow((_xmin - node[node_id].X) * (_xmax - _xmin) + (_ymin - node[node_id].Y) * (_ymax - _ymin) + (_zmin - node[node_id].Z) * (_zmax - _zmin),2)) / (std::pow(_xmax - _xmin,2) + std::pow(_ymax - _ymin,2) + std::pow(_zmax - _zmin,2))) < 0.1*mesh_spacing)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),6);
          }
        }
        boundary_info.nodeset_name(0) = "Left";
        boundary_info.nodeset_name(1) = "Right";
        boundary_info.nodeset_name(2) = "Bottom";
        boundary_info.nodeset_name(3) = "Top";
        boundary_info.nodeset_name(4) = "Back";
        boundary_info.nodeset_name(5) = "Front";
        boundary_info.nodeset_name(6) = "Diag";
      }
      else if (_shape == 2) // cylinder shape
      {
        mesh_spacing = 2.0 * _R / (2 * _nx - 2);
        nz = static_cast<int>((_zmax - _zmin) / mesh_spacing) + 1;
        horizon = mesh_spacing * 3.0;
        // Create and Initialize Node Structure
        node_per_layer = 1;
        for (i = 1; i < _nx; i++)
        {
          j = std::floor(3.14159265359 / std::asin(1.0 / (2.0 * i)) + 0.5);
          node_per_layer += j;
        }
        node_num = node_per_layer * nz;
        node = (struct node_structure3D*)malloc(node_num * sizeof(struct node_structure3D));
        InitializeNode3D(node, node_num);
        mesh.reserve_nodes(node_num);
        // Define Nodal Coordinates
        for (k = 0; k < nz; k++)
        {
          Z = _zmin + k * mesh_spacing;
          mesh.add_point (Point(0.0, 0.0, Z), node_id);
          node[node_id].X = 0;
          node[node_id].Y = 0;
          node[node_id].Z = Z;
          node_id++;
          for (i = 1; i < _nx; i++)
          {
            n = std::floor(3.14159265359 / std::asin(1.0 / (2.0 * i)) + 0.5);
            for (j = 0; j < n; j++)
            {
              X = i * mesh_spacing * std::cos(j * 2.0 * 3.14159265359 / n);
              Y = i * mesh_spacing * std::sin(j * 2.0 * 3.14159265359 / n);
              Z = _zmin + k * mesh_spacing;
              mesh.add_point (Point(X, Y, Z), node_id);
              node[node_id].X = X;
              node[node_id].Y = Y;
              node[node_id].Z = Z;
              node_id++;
            }
          }
        }
        // Search Family Member
        search_range = 4 * node_per_layer;
        SearchBond3D(node, node_num, search_range, horizon);
        // Generate Mesh
        for(i = 0, j = 0; i < node_num; i++)
        {
          j += node[i].bonds_per_node;
        }
        bond_num = j / 2;
        mesh.reserve_elem (bond_num);
        for (i = 0; i < node_num; i++)
        {
          for(j = 0; j < node[i].bonds_per_node; j++)
          {
            k = node[i].bond[j].node_index;
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
        for (node_id = 0; node_id < node_num; node_id++)
        {
          X = node[node_id].X;
          Y = node[node_id].Y;
          Z = node[node_id].Z;
          dis = std::sqrt(X * X + Y * Y);
          if (dis > _R - mesh_spacing)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),0);
          }
          if (dis < 0.001 * mesh_spacing)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),1);
          }
          if (dis < 0.001 * mesh_spacing && std::abs(Z - (_zmax + _zmin) / 2) < mesh_spacing)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),2);
          }
          if (std::abs(Y) < 0.001 * mesh_spacing && dis > _R - mesh_spacing && X < 0.0)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),3);
          }
          if (std::abs(Y) < 0.001 * mesh_spacing && dis > _R - mesh_spacing && X > 0.0)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),4);
          }
          if (std::abs(X) < 0.001 * mesh_spacing && dis > _R - mesh_spacing && Y < 0.0)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),5);
          }
          if (std::abs(X) < 0.001 * mesh_spacing && dis > _R - mesh_spacing && Y > 0.0)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),6);
          }
          if (Z < _zmin + mesh_spacing)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),7);
          }
          if (Z > _zmax - mesh_spacing)
          {
            boundary_info.add_node(mesh.node_ptr(node_id),8);
          }
        }
        boundary_info.nodeset_name(0) = "Periphery";
        boundary_info.nodeset_name(1) = "CenterLine";
        boundary_info.nodeset_name(2) = "Center";
        boundary_info.nodeset_name(3) = "LeftLine";
        boundary_info.nodeset_name(4) = "RightLine";
        boundary_info.nodeset_name(5) = "BackLine";
        boundary_info.nodeset_name(6) = "FrontLine";
        boundary_info.nodeset_name(7) = "Bottom";
        boundary_info.nodeset_name(8) = "Top";
      }
    }
    std::cout << "Total Node Number: " << node_num << std::endl;
    std::cout << "Total Bond Number: " << bond_num << std::endl;

    // Prepare for use 
    mesh.prepare_for_use (/*skip_renumber =*/ false);
}
