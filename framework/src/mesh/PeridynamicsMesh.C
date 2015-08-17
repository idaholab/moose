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
	}bond[122];
};

int CountNodeNum(double R,double dx,double dy,int nd)
{
	int NodeNum,i,j;
	double dist,X,Y;
	for (j=0,NodeNum=0; j<nd; j++)
	{
		for (i=0; i<nd; i++)
		{
			X = -R + dx / 2.0 + static_cast<Real>(i)*dx;
			Y = -R + dy / 2.0 + static_cast<Real>(j)*dy;
			dist = sqrt(X*X + Y*Y);
			if (dist <= R + 0.001*dx)
			{
				NodeNum++;
			}
		}
	}
	return NodeNum;
}

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
		}
	}
}

void SearchBond2D(struct node_structure2D *Node,int NodeNum,int NodeNumX,double Horizon,double MeshSpacing)
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
				if(k>=0 && k<NodeNum && k != nodeid)
				{
					d = sqrt(pow(Node[nodeid].X-Node[k].X,2) + pow(Node[nodeid].Y-Node[k].Y,2));
					if(d <= Horizon + 0.001*MeshSpacing)
					{
						Node[nodeid].bond[num].NodesIndex = k;
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
			}
			else if(i == 1)
			{
				k = nodeid + 3;
			}
			else if(i == 2)
			{
				k = nodeid - 3 * NodeNumX;
			}
			else
			{
				k = nodeid + 3 * NodeNumX;
			}
			if(k>=0 && k<NodeNum)
			{
				d = sqrt(pow(Node[nodeid].X-Node[k].X,2) + pow(Node[nodeid].Y-Node[k].Y,2));
				if(d <= Horizon + 0.001*MeshSpacing)
				{
					Node[nodeid].bond[num].NodesIndex = k;
					num++;
				}
			}
		}
		Node[nodeid].BondsNumPerNode = num;
	}
}

void SearchBondGeneral2D(struct node_structure2D *Node,int NodeNum,int Range,double Horizon,double MeshSpacing)
{
	int i,j,k,num,nodeid;
	double d;
	for(nodeid=0;nodeid<NodeNum;nodeid++)
	{
		num = 0;
		i = (nodeid - Range >= 0)? nodeid - Range:0;
		j = (nodeid + Range <= NodeNum)? nodeid + Range:NodeNum;
		for(k = i;k < j;k++)
		{
			d = sqrt(pow(Node[nodeid].X-Node[k].X,2) + pow(Node[nodeid].Y-Node[k].Y,2));
			if(d <= Horizon + 0.001*MeshSpacing && k != nodeid)
			{
				Node[nodeid].bond[num].NodesIndex = k;
				num++;
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
			}
			else if(i == 1)
			{
				n = nodeid + 3;
			}
			else if(i == 2)
			{
				n = nodeid - 3 * NodeNumX;
			}
			else if(i == 3)
			{
				n = nodeid + 3 * NodeNumX;
			}
			else if(i == 4)
			{
				n = nodeid - 3 * NodeNumX * NodeNumY;
			}
			else if(i == 5)
			{
				n = nodeid + 3 * NodeNumX * NodeNumY;
			}
			if(n>=0 && n<NodeNum)
			{
				d = sqrt(pow(Node[nodeid].X-Node[n].X,2) + pow(Node[nodeid].Y-Node[n].Y,2) + pow(Node[nodeid].Z-Node[n].Z,2));
				if(d <= Horizon + 0.001)
				{
					Node[nodeid].bond[num].NodesIndex = n;
					num++;
				}
			}
		}
		Node[nodeid].BondsNumPerNode = num;
	}
}

void SearchBondGeneral3D(struct node_structure3D *Node,int NodeNum,int Range,double Horizon,double MeshSpacing)
{
	int i,j,k,num,nodeid;
	double d;
	for(nodeid=0;nodeid<NodeNum;nodeid++)
	{
		num = 0;
		i = (nodeid - Range >= 0)? nodeid - Range:0;
		j = (nodeid + Range <= NodeNum)? nodeid + Range:NodeNum;
		for(k = i;k < j;k++)
		{
			d = sqrt(pow(Node[nodeid].X-Node[k].X,2) + pow(Node[nodeid].Y-Node[k].Y,2) + pow(Node[nodeid].Z-Node[k].Z,2));
			if(d <= Horizon + 0.001*MeshSpacing && k != nodeid)
			{
				Node[nodeid].bond[num].NodesIndex = k;
				num++;
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
  params.addParam<int>("shape", 1, "1. Rectangular, 2. Disk");
  params.addRangeCheckedParam<int>("nx", 1, "nx>0", "Number of elements in the X direction");
  params.addRangeCheckedParam<int>("ny", 1, "ny>=0", "Number of elements in the Y direction");
  params.addRangeCheckedParam<int>("nz", 1, "nz>=0", "Number of elements in the Z direction");
	params.addRangeCheckedParam<int>("nr", 1, "nr>=0", "Number of elements in the Z direction");
  params.addParam<Real>("xmin", 0.0, "Lower X Coordinate of the generated mesh");
  params.addParam<Real>("ymin", 0.0, "Lower Y Coordinate of the generated mesh");
  params.addParam<Real>("zmin", 0.0, "Lower Z Coordinate of the generated mesh");
  params.addParam<Real>("xmax", 1.0, "Upper X Coordinate of the generated mesh");
  params.addParam<Real>("ymax", 1.0, "Upper Y Coordinate of the generated mesh");
  params.addParam<Real>("zmax", 1.0, "Upper Z Coordinate of the generated mesh");
	params.addParam<Real>("R", 1.0, "Radius of the generated mesh");
  params.addParam<MooseEnum>("elem_type", elem_types, "The type of element from libMesh to generate (default: linear element for requested dimension)");

  params.addParamNamesToGroup("dim", "Main");

  return params;
}

PeridynamicsMesh::PeridynamicsMesh(const InputParameters & parameters) :
    MooseMesh(parameters),
    _dim(getParam<MooseEnum>("dim")),
    _nx(getParam<int>("nx")),
    _ny(getParam<int>("ny")),
    _nz(getParam<int>("nz")),
		_nr(getParam<int>("nr")),
		_shape(getParam<int>("shape"))
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
  double _xmin,_ymin,_zmin,_xmax,_ymax,_zmax,_R;
	double MeshSpacingX,MeshSpacingY,MeshSpacingZ,Horizon,dist,X,Y,Z;
  unsigned int i,j,k,m,node_id;
  unsigned int NodeNum,NodeNumPerLayer,BondNum,nd,nz,Range;
  Point mypoint(0.0,0.0,0.0);
  //cout << "shape = " << _shape << endl;
  if (_dim == 2)
  {
		struct node_structure2D *Node;
		if (_shape == 1)
		{
			_xmin = getParam<Real>("xmin");
	  	_ymin = getParam<Real>("ymin");
	  	_xmax = getParam<Real>("xmax");
	  	_ymax = getParam<Real>("ymax");
			MeshSpacingX = (_xmax - _xmin) / _nx;
	  	MeshSpacingY = (_ymax - _ymin) / _ny;
	  	Horizon = MeshSpacingX*3.0;
	  	// Create and Initialize Node Structure //
	  	NodeNum = _nx*_ny;
	  	Node = (struct node_structure2D*)malloc(NodeNum*sizeof(struct node_structure2D));
	  	InitializeNode2D(Node,NodeNum);
			mesh.reserve_nodes(NodeNum);
	  	// Define Nodal Coordinates //
	  	for (j=0,node_id=0; j<_ny; j++)
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
	  	SearchBond2D(Node,NodeNum,_nx,Horizon,MeshSpacingX);
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
	  		for (j=0; j<1; j++)
	  		{
	  			k = i + j*_nx;
	  			boundary_info.add_node(mesh.node_ptr(k),0);
	  			k = NodeNum - 1*_nx + i + j*_nx;
	  			boundary_info.add_node(mesh.node_ptr(k),1);
	  		}
	  	}
	  	boundary_info.nodeset_name(0) = "Bottom";
	  	boundary_info.nodeset_name(1) = "Top";
		}
		else if (_shape == 2)
		{
			_R = getParam<Real>("R");
			nd = 2 * _nr + 1;
			MeshSpacingX = 2*_R / nd;
	  	MeshSpacingY = MeshSpacingX;
			Horizon = MeshSpacingX*3.0;
			// Create and Initialize Node Structure //
			NodeNum = CountNodeNum(_R,MeshSpacingX,MeshSpacingY,nd);
			Node = (struct node_structure2D*)malloc(NodeNum*sizeof(struct node_structure2D));
	  	InitializeNode2D(Node,NodeNum);
			mesh.reserve_nodes(NodeNum);
			// Define Nodal Coordinates //
			for (j=0,node_id=0; j<nd; j++)
	  	{
	  		for (i=0; i<nd; i++)
	  		{
					X = -_R + MeshSpacingX / 2.0 + static_cast<Real>(i)*MeshSpacingX;
					Y = -_R + MeshSpacingY / 2.0 + static_cast<Real>(j)*MeshSpacingY;
					dist = sqrt(X*X + Y*Y);
					if (dist <= _R + 0.001*MeshSpacingX)
					{
						mypoint = Point(X,Y,0.0);
						mesh.add_point (mypoint, node_id);
						Node[node_id].X = X;
						Node[node_id].Y = Y;
						node_id++;
					}
				}
			}
			// Search Family Member //
			Range = 3 * nd + 1;
	  	SearchBondGeneral2D(Node,NodeNum,Range,Horizon,MeshSpacingX);
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
	    for (node_id=0; node_id<NodeNum; node_id++)
	  	{
				X = Node[node_id].X;
				Y = Node[node_id].Y;
				dist = sqrt(X*X + Y*Y);
				if (dist >= _R - MeshSpacingX)
				{
					boundary_info.add_node(mesh.node_ptr(node_id),0);
				}
				if (abs(dist) < 0.00001*MeshSpacingX)
				{
					boundary_info.add_node(mesh.node_ptr(node_id),1);
				}
				if (abs(Y) < 0.00001*MeshSpacingY && dist >= _R - MeshSpacingX && X > 0.0)
				{
					boundary_info.add_node(mesh.node_ptr(node_id),2);
				}
				boundary_info.add_node(mesh.node_ptr(node_id),3);
	  	}
	  	boundary_info.nodeset_name(0) = "Outside";
	  	boundary_info.nodeset_name(1) = "CenterPoint";
			boundary_info.nodeset_name(2) = "SidePoint";
			boundary_info.nodeset_name(3) = "BottomPoint";
		}
 	}
 	else if(_dim == 3)
 	{
 		struct node_structure3D *Node;
		if (_shape == 1)
		{
			_xmin = getParam<Real>("xmin");
	  	_ymin = getParam<Real>("ymin");
	  	_zmin = getParam<Real>("zmin");
	  	_xmax = getParam<Real>("xmax");
	  	_ymax = getParam<Real>("ymax");
	  	_zmax = getParam<Real>("zmax");
			MeshSpacingX = (_xmax - _xmin) / _nx;
	  	MeshSpacingY = (_ymax - _ymin) / _ny;
	  	MeshSpacingZ = (_zmax - _zmin) / _nz;
	  	Horizon = MeshSpacingX*3.0;
			// Create and Initialize Node Structure //
			NodeNum = _nx*_ny*_nz;
	  	Node = (struct node_structure3D*)malloc(NodeNum*sizeof(struct node_structure3D));
	  	InitializeNode3D(Node,NodeNum);
			mesh.reserve_nodes(NodeNum);
			// Define Nodal Coordinates //
	  	for(k=0,node_id=0; k<_nz; k++)
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
	  			for (k=0; k<1; k++)
	  			{
	  				m = i + j*_nx + k*_nx*_ny;
	  				boundary_info.add_node(mesh.node_ptr(m),0);
	  				m = NodeNum - 1*_nx*_ny + i + j*_nx + k*_nx*_ny;
	  				boundary_info.add_node(mesh.node_ptr(m),1);
	  			}
	  		}
	  	}
	  	boundary_info.nodeset_name(0) = "Bottom";
	  	boundary_info.nodeset_name(1) = "Top";
		}
		else if (_shape == 2)
		{
			_R = getParam<Real>("R");
			_zmin = getParam<Real>("zmin");
	  	_zmax = getParam<Real>("zmax");
			nd = 2 * _nr + 1;
			MeshSpacingX = 2*_R / nd;
	  	MeshSpacingY = MeshSpacingX;
			MeshSpacingZ = MeshSpacingX;
			Horizon = MeshSpacingX*3.0;
			nz = static_cast<int>((_zmax - _zmin) / MeshSpacingX);
			// Create and Initialize Node Structure //
			NodeNumPerLayer = CountNodeNum(_R,MeshSpacingX,MeshSpacingY,nd);
      NodeNum = NodeNumPerLayer * nz;
			Node = (struct node_structure3D*)malloc(NodeNum*sizeof(struct node_structure3D));
	  	InitializeNode3D(Node,NodeNum);
			mesh.reserve_nodes(NodeNum);
			// Define Nodal Coordinates //
			for (k=0,node_id=0; k<nz; k++)
			{
				for (j=0; j<nd; j++)
		  	{
		  		for (i=0; i<nd; i++)
		  		{
						X = -_R + MeshSpacingX / 2.0 + static_cast<Real>(i)*MeshSpacingX;
						Y = -_R + MeshSpacingY / 2.0 + static_cast<Real>(j)*MeshSpacingY;
						Z = _zmin+MeshSpacingZ / 2.0 + static_cast<Real>(k)*MeshSpacingZ;
						dist = sqrt(X*X + Y*Y);
						if (dist <= _R + 0.001*MeshSpacingX)
						{
							mypoint = Point(X,Y,Z);
							mesh.add_point (mypoint, node_id);
							Node[node_id].X = X;
							Node[node_id].Y = Y;
							Node[node_id].Z = Z;
							node_id++;
						}
					}
				}
			}
			// Search Family Member //
			Range = 3 * NodeNumPerLayer + 1;
	  	SearchBondGeneral3D(Node,NodeNum,Range,Horizon,MeshSpacingX);
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
	    for (node_id=0; node_id<NodeNum; node_id++)
	  	{
				X = Node[node_id].X;
				Y = Node[node_id].Y;
				Z = Node[node_id].Z;
				dist = sqrt(X*X + Y*Y);
				if (dist >= _R - MeshSpacingX)
				{
					boundary_info.add_node(mesh.node_ptr(node_id),0);
				}
				if (abs(dist) < 0.00001*MeshSpacingX)
				{
					boundary_info.add_node(mesh.node_ptr(node_id),1);
				}
				if (abs(Y) < 0.00001*MeshSpacingY && dist >= _R - MeshSpacingX && X > 0.0)
				{
					boundary_info.add_node(mesh.node_ptr(node_id),2);
				}
				if (abs(dist) < 0.00001*MeshSpacingX && Z < _zmin+MeshSpacingZ)
				{
					boundary_info.add_node(mesh.node_ptr(node_id),3);
				}
	  	}
	  	boundary_info.nodeset_name(0) = "Outside";
	  	boundary_info.nodeset_name(1) = "CenterPoint";
			boundary_info.nodeset_name(2) = "SidePoint";
			boundary_info.nodeset_name(3) = "BottomPoint";
		}
 	}
	cout << "NodeNum = " << NodeNum << endl;
	cout << "BondNum = " << BondNum << endl;
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

//DEPRECATED CONSTRUCTOR
PeridynamicsMesh::PeridynamicsMesh(const std::string & deprecated_name, InputParameters parameters) :
    MooseMesh(deprecated_name, parameters),
    _dim(getParam<MooseEnum>("dim")),
    _nx(getParam<int>("nx")),
    _ny(getParam<int>("ny")),
    _nz(getParam<int>("nz")),
		_nr(getParam<int>("nr")),
		_shape(getParam<int>("shape"))
{
}
