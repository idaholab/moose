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
#include "libmesh/cell_hex8.h"
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
		int NodesOrder;
		double Length;
		double C;
	}bond[28];
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
		int NodesOrder;
		double Length;
		double C;
	}bond[122];
};

void InitializeNode2D(struct node_structure2D *Node,int NodeNum)
{
	int i,j;
	for(i = 0;i < NodeNum;i++)
	{
		Node[i].X = 0.0;
		Node[i].Y = 0.0;
		Node[i].BondsNumPerNode = 0;
		for(j = 0;j < 28;j++)
		{
			Node[i].bond[j].NodesIndex = 0;
			Node[i].bond[j].NodesOrder = 0;
			Node[i].bond[j].Length = 0.0;
			Node[i].bond[j].C = 0.0;
		}
	}
}

void InitializeNode3D(struct node_structure3D *Node,int NodeNum)
{
	int i,j;
	for(i = 0;i < NodeNum;i++)
	{
		Node[i].X = 0.0;
		Node[i].Y = 0.0;
		Node[i].Z = 0.0;
		Node[i].BondsNumPerNode = 0;
		for(j = 0;j < 122;j++)
		{
			Node[i].bond[j].NodesIndex = 0;
			Node[i].bond[j].NodesOrder = 0;
			Node[i].bond[j].Length = 0.0;
			Node[i].bond[j].C = 0.0;
		}
	}
}

void SearchBond2D(struct node_structure2D *Node,int NodeNum,int NodeNumX,double Horizon)
{
	int i,j,k,m,num,nodeid;
	double d;
	for(nodeid = 0;nodeid<NodeNum;nodeid++)
	{
		num = 0;
		for(i = -2;i < 3;i++)
		{
			for(j = -2;j < 3;j++)
			{
				k = nodeid + i + j * NodeNumX;
				if(j < 0)
				{
					m = 3 + i + (2+j)*5;
				}
				else if(j == 0 && i < 0)
				{
					m = 14 + i;
				}
				else if(j == 0 && i > 0)
				{
					m = 13 + i;
				}
				else
				{
					m = 19 + i + (j-1)*5;
				}
				if(k>=0 && k<NodeNum && k != nodeid)
				{
					d = sqrt(pow(Node[nodeid].X-Node[k].X,2) + pow(Node[nodeid].Y-Node[k].Y,2));
					if(d <= Horizon + 0.001)
					{
						Node[nodeid].bond[num].NodesIndex = k;
						Node[nodeid].bond[num].NodesOrder = m;
						num++;
					}
				}
			}
		}
		for(i = 0;i<4;i++)
		{
			if(i == 0)
			{
				k = nodeid - 3;
				m = 11;
			}
			else if(i == 1)
			{
				k = nodeid + 3;
				m = 16;
			}
			else if(i == 2)
			{
				k = nodeid - 3 * NodeNumX;
				m = 0;
			}
			else
			{
				k = nodeid + 3 * NodeNumX;
				m = 27;
			}
			if(k>=0 && k<NodeNum)
			{
				d = sqrt(pow(Node[nodeid].X-Node[k].X,2) + pow(Node[nodeid].Y-Node[k].Y,2));
				if(d <= Horizon + 0.001)
				{
					Node[nodeid].bond[num].NodesIndex = k;
					Node[nodeid].bond[num].NodesOrder = m;
					num++;
				}
			}
		}
		Node[nodeid].BondsNumPerNode = num;
	}
}

void SearchBond3D(struct node_structure3D *Node,int NodeNum,int NodeNumX,int NodeNumY,double Horizon)
{
	int i,j,k,m,n,num = 0,nodeid;
	double d;
	for(nodeid = 0;nodeid<NodeNum;nodeid++)
	{
		num = 0;
		for(i = -2;i < 3;i++)
		{
			for(j = -2;j < 3;j++)
			{
				for(k = -2;k < 3;k++)
				{
					n = nodeid + i + j * NodeNumX + k * NodeNumX * NodeNumY;
					if(n>=0 && n<NodeNum && n != nodeid)
					{
						d = sqrt(pow(Node[nodeid].X-Node[n].X,2) + pow(Node[nodeid].Y-Node[n].Y,2) + pow(Node[nodeid].Z-Node[n].Z,2));
						if(d <= Horizon + 0.001)
						{
							Node[nodeid].bond[num].NodesIndex = n;
							if(k == -2 && j == -2)
							{
								m = i + 2;
								Node[nodeid].bond[num].NodesOrder = m;
							}
							else if(k == -2 && j == 2)
							{
								m = i + 20;
								Node[nodeid].bond[num].NodesOrder = m;
							}
							else if(k == 2 && j == -2)
							{
								m = i + 101;
								Node[nodeid].bond[num].NodesOrder = m;
							}
							else if(k == 2 && j == 2)
							{
								m = i + 119;
								Node[nodeid].bond[num].NodesOrder = m;
							}
							else if(k == -2 && j > -2 && j < 2)
							{
								m = i + (j+1)*5 + 6;
								Node[nodeid].bond[num].NodesOrder = m;
							}
							else if(k == 2 && j > -2 && j < 2)
							{
								m = i + (j+1)*5 + 105;
								Node[nodeid].bond[num].NodesOrder = m;
							}
							else if(k == -1)
							{
								m = i + (j+2)*5 + 24;
								Node[nodeid].bond[num].NodesOrder = m;
							}
							else if(k == 1)
							{
								m = i + (j+2)*5 + 77;
								Node[nodeid].bond[num].NodesOrder = m;
							}
							else if(k == 0 && j < 0)
							{
								m = i + (j+2)*5 + 50;
								Node[nodeid].bond[num].NodesOrder = m;
							}
							else if(k == 0 && j > 0)
							{
								m = i + (j-1)*5 + 66;
								Node[nodeid].bond[num].NodesOrder = m;
							}
							else if(k == 0 && j == 0 && i < 0)
							{
								m = i + 61;
								Node[nodeid].bond[num].NodesOrder = m;
							}
							else if(k == 0 && j == 0 && i > 0)
							{
								m = i + 60;
								Node[nodeid].bond[num].NodesOrder = m;
							}
							num++;
						}
					}
				}
			}
		}
		for(i = 0;i<6;i++)
		{
			if(i == 0)
			{
				n = nodeid - 3;
				m = 58;
			}
			else if(i == 1)
			{
				n = nodeid + 3;
				m = 63;
			}
			else if(i == 2)
			{
				n = nodeid - 3 * NodeNumX;
				m = 47;
			}
			else if(i == 3)
			{
				n = nodeid + 3 * NodeNumX;
				m = 74;
			}
			else if(i == 4)
			{
				n = nodeid - 3 * NodeNumX * NodeNumY;
				m = 0;
			}
			else if(i == 5)
			{
				n = nodeid + 3 * NodeNumX * NodeNumY;
				m = 121;
			}
			if(n>=0 && n<NodeNum)
			{
				d = sqrt(pow(Node[nodeid].X-Node[n].X,2) + pow(Node[nodeid].Y-Node[n].Y,2) + pow(Node[nodeid].Z-Node[n].Z,2));
				if(d <= Horizon + 0.001)
				{
					Node[nodeid].bond[num].NodesIndex = n;
					Node[nodeid].bond[num].NodesOrder = m;
					num++;
				}
			}
		}
		Node[nodeid].BondsNumPerNode = num;
	}
}

/***********************************************************************************************/
template<>
InputParameters validParams<PeridynamicsMesh>()
{
  InputParameters params = validParams<MooseMesh>();

  MooseEnum elem_types("EDGE EDGE2 EDGE3 EDGE4 QUAD QUAD4 QUAD8 QUAD9 TRI3 TRI6 HEX HEX8 HEX20 HEX27 TET4 TET10 PRISM6 PRISM15 PRISM18 PYRAMID5 PYRAMID13 PYRAMID14"); // no default

  MooseEnum dims("1=1 2 3");
  params.addRequiredParam<MooseEnum>("dim", dims, "The dimension of the mesh to be generated"); // Make this parameter required

  params.addRangeCheckedParam<int>("nx", 1, "nx>0", "Number of elements in the X direction");
  params.addRangeCheckedParam<int>("ny", 1, "ny>=0", "Number of elements in the Y direction");
  params.addRangeCheckedParam<int>("nz", 1, "nz>=0", "Number of elements in the Z direction");
  params.addParam<Real>("xmin", 0.0, "Lower X Coordinate of the generated mesh");
  params.addParam<Real>("ymin", 0.0, "Lower Y Coordinate of the generated mesh");
  params.addParam<Real>("zmin", 0.0, "Lower Z Coordinate of the generated mesh");
  params.addParam<Real>("xmax", 1.0, "Upper X Coordinate of the generated mesh");
  params.addParam<Real>("ymax", 1.0, "Upper Y Coordinate of the generated mesh");
  params.addParam<Real>("zmax", 1.0, "Upper Z Coordinate of the generated mesh");
  params.addParam<MooseEnum>("elem_type", elem_types, "The type of element from libMesh to generate (default: linear element for requested dimension)");

  params.addParamNamesToGroup("dim", "Main");

  return params;
}

PeridynamicsMesh::PeridynamicsMesh(const std::string & name, InputParameters parameters) :
    MooseMesh(name, parameters),
    _dim(getParam<MooseEnum>("dim")),
    _nx(getParam<int>("nx")),
    _ny(getParam<int>("ny")),
    _nz(getParam<int>("nz"))
{
}

PeridynamicsMesh::PeridynamicsMesh(const PeridynamicsMesh & other_mesh) :
    MooseMesh(other_mesh),
    _dim(other_mesh._dim),
    _nx(other_mesh._nx),
    _ny(other_mesh._ny),
    _nz(other_mesh._nz)
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
  MooseEnum elem_type_enum = getParam<MooseEnum>("elem_type");

  if (!isParamValid("elem_type"))
  {
    // Switching on MooseEnum
    switch (_dim)
    {
    case 1: elem_type_enum = "EDGE2"; break;
    case 2: elem_type_enum = "QUAD4"; break;
    case 3: elem_type_enum = "HEX8"; break;
    }
  }

  ElemType elem_type = Utility::string_to_enum<ElemType>(elem_type_enum);
  UnstructuredMesh& mesh = dynamic_cast<UnstructuredMesh&>(getMesh());
  mesh.clear();
  mesh.set_mesh_dimension(1);
  BoundaryInfo& boundary_info = mesh.get_boundary_info();
  //libmesh_assert_not_equal_to (1, 0);
  //libmesh_assert_equal_to (0, 0);
  //libmesh_assert_equal_to (0, 0);
  //libmesh_assert_less (0., 1.);
  /***********************************************************************************************/
  /* Peridynamic Mesh */
  /***********************************************************************************************/	
  double _xmin,_ymin,_zmin,_xmax,_ymax,_zmax;
  unsigned int i,j,k,m,node_id = 0;
  unsigned int NodeNum,BondNum;
  Point mypoint(0.0,0.0,0.0);
       
  if (_dim == 2)
  {
  	_xmin = getParam<Real>("xmin");
  	_ymin = getParam<Real>("ymin");
  	_xmax = getParam<Real>("xmax");
  	_ymax = getParam<Real>("ymax");
  	// Create and Initialize Node Structure //
  	struct node_structure2D *Node;
  	NodeNum = _nx*_ny;
  	Node = (struct node_structure2D*)malloc(NodeNum*sizeof(struct node_structure2D));
  	InitializeNode2D(Node,NodeNum);
  	// Define Nodal Coordinates //
  	double MeshSpacingX = (_xmax - _xmin) / _nx;
  	double MeshSpacingY = (_ymax - _ymin) / _ny;
  	double Horizon = MeshSpacingX*3.0;
  	mesh.reserve_nodes(NodeNum);
  	for (j=0; j<_ny; j++)
  	{
  		for (i=0; i<_nx; i++)
  		{
  			mypoint = Point(MeshSpacingX / 2.0 + static_cast<Real>(i)*MeshSpacingX,
  			                MeshSpacingY / 2.0 + static_cast<Real>(j)*MeshSpacingY,
  			                0.0);
  			mesh.add_point (mypoint, node_id);
  			Node[node_id].X = MeshSpacingX / 2.0 + static_cast<Real>(i)*MeshSpacingX;
  			Node[node_id].Y = MeshSpacingY / 2.0 + static_cast<Real>(j)*MeshSpacingY;
  			node_id++;
      }
    }
    // Search Family Member //
  	SearchBond2D(Node,NodeNum,_nx,Horizon);
  	// Generate Mesh //
  	for(i=0,j=0; i<NodeNum; i++)
  	{
  		j += Node[i].BondsNumPerNode;
  	}
  	BondNum = j / 2;
    mesh.reserve_elem (BondNum);
    for (i=0,m=0; i<NodeNum; i++)
    {
      for(j=0; j<Node[i].BondsNumPerNode; j++)
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
    // Define Boundary Nodes //
    for (i=0; i<_nx; i++)
  	{
  		for (j=0; j<3; j++)
  		{
  			k = i + j*_nx;
  			boundary_info.add_node(mesh.node_ptr(k),0);
  			k = NodeNum - 3*_nx + i + j*_nx;
  			boundary_info.add_node(mesh.node_ptr(k),1);
  		}
  	}
  	boundary_info.nodeset_name(0) = "Bottom";
  	boundary_info.nodeset_name(1) = "Top";
 	}
 	else if(_dim == 3)
 	{
 		
 		_xmin = getParam<Real>("xmin");
  	_ymin = getParam<Real>("ymin");
  	_zmin = getParam<Real>("zmin");
  	_xmax = getParam<Real>("xmax");
  	_ymax = getParam<Real>("ymax");
  	_zmax = getParam<Real>("zmax");
  	// Create and Initialize Node Structure //
  	struct node_structure3D *Node;
  	NodeNum = _nx*_ny*_nz;
  	Node = (struct node_structure3D*)malloc(NodeNum*sizeof(struct node_structure3D));
  	InitializeNode3D(Node,NodeNum);
  	// Define Nodal Coordinates //
  	double MeshSpacingX = (_xmax - _xmin) / _nx;
  	double MeshSpacingY = (_ymax - _ymin) / _ny;
  	double MeshSpacingZ = (_zmax - _zmin) / _nz;
  	double Horizon = MeshSpacingX*3.0;
  	mesh.reserve_nodes(NodeNum);
  	for(k=0; k<_nz; k++)
  	{
  		for (j=0; j<_ny; j++)
  		{
  			for (i=0; i<_nx; i++)
  			{
  				mypoint = Point(MeshSpacingX / 2.0 + static_cast<Real>(i)*MeshSpacingX,
  			                  MeshSpacingY / 2.0 + static_cast<Real>(j)*MeshSpacingY,
  			                  MeshSpacingZ / 2.0 + static_cast<Real>(k)*MeshSpacingZ);
  			  mesh.add_point (mypoint, node_id);
  			  Node[node_id].X = MeshSpacingX / 2.0 + static_cast<Real>(i)*MeshSpacingX;
  			  Node[node_id].Y = MeshSpacingY / 2.0 + static_cast<Real>(j)*MeshSpacingY;
  			  Node[node_id].Z = MeshSpacingZ / 2.0 + static_cast<Real>(k)*MeshSpacingZ;
  			  node_id++;
        }
      }
 	  }
 	  // Search Family Member //
  	SearchBond3D(Node,NodeNum,_nx,_ny,Horizon);
  	// Generate Mesh //
  	for(i=0,j=0; i<NodeNum; i++)
  	{
  		j += Node[i].BondsNumPerNode;
  	}
  	BondNum = j / 2;
    mesh.reserve_elem (BondNum);
    for (i=0; i<NodeNum; i++)
    {
      for(j=0; j<Node[i].BondsNumPerNode; j++)
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
    // Define Boundary Nodes //
    for (i=0; i<_nx; i++)
  	{
  		for (j=0; j<_ny; j++)
  		{
  			for (k=0; k<3; k++)
  			{
  				m = i + j*_nx + k*_nx*_ny;
  				boundary_info.add_node(mesh.node_ptr(m),0);
  				m = NodeNum - 3*_nx*_ny + i + j*_nx + k*_nx*_ny;
  				boundary_info.add_node(mesh.node_ptr(m),1);
  			}
  		}
  	}
  	boundary_info.nodeset_name(0) = "Bottom";
  	boundary_info.nodeset_name(1) = "Top";
 	}
 	// Define a Dummy Hex//
 	mesh.add_point (Point(-2.,-2.,-2.), node_id+0);
 	mesh.add_point (Point(-1.,-2.,-2.), node_id+1);
 	mesh.add_point (Point(-1.,-1.,-2.), node_id+2);
 	mesh.add_point (Point(-2.,-1.,-2.), node_id+3);
 	mesh.add_point (Point(-2.,-2.,-1.), node_id+4);
 	mesh.add_point (Point(-1.,-2.,-1.), node_id+5);
 	mesh.add_point (Point(-1.,-1.,-1.), node_id+6);
 	mesh.add_point (Point(-2.,-1.,-1.), node_id+7);
 	Elem* elem = mesh.add_elem(new Hex8);
 	for(i=0; i<8; i++)
 	{
 		elem->set_node(i) = mesh.node_ptr(node_id+i);
 		boundary_info.add_node(mesh.node_ptr(node_id+i),100);
 	}
 	boundary_info.nodeset_name(100) = "Hex"; 
  elem->subdomain_id() = 100;
 	// Prepare for use //
  mesh.prepare_for_use (/*skip_renumber =*/ false);
  
  /***********************************************************************************************/
}
