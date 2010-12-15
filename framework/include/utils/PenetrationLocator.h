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

#ifndef PENETRATIONLOCATOR_H
#define PENETRATIONLOCATOR_H

// libmesh includes
#include "libmesh_common.h"
#include "mesh.h"
#include "vector_value.h"
#include "point.h"

#include <vector>
#include <map>

//Forward Declarations
class MooseSystem;

class PenetrationLocator
{
public:

  PenetrationLocator(MooseSystem & moose_system, Mesh & mesh, unsigned int master, unsigned int slave);
  void detectPenetration();

  Real penetrationDistance(unsigned int node_id);
  RealVectorValue penetrationNormal(unsigned int node_id);
  
  /**
   * Data structure used to hold penetation information
   */
  class PenetrationInfo 
  {
  public:
    PenetrationInfo(Node * node, Elem * elem, Elem * side, RealVectorValue norm, Real norm_distance, const Point & closest_point);

    PenetrationInfo(const PenetrationInfo & p);

    ~PenetrationInfo();
    Node * _node;
    Elem * _elem;
    Elem * _side;
    RealVectorValue _normal;
    Real _distance;
    Point _closest_point;
  };

  MooseSystem & _moose_system;

  Real normDistance(const Elem & elem, const Elem & side, const Point & p0, Point & closest_point, RealVectorValue & normal);

  int intersect2D_Segments( Point S1P0, Point S1P1, Point S2P0, Point S2P1, Point* I0, Point* I1 );
  int inSegment(Point P, Point SP0, Point SP1);
  
  Mesh & _mesh;
  unsigned int _master_boundary;
  unsigned int _slave_boundary;

  /**
   * Data structure of nodes and their associated penetration information
   */
  std::map<unsigned int, PenetrationInfo *> _penetration_info;

  std::map<unsigned int, bool> _has_penetrated;
};


#endif //PENETRATIONLOCATOR_H
