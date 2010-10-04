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

#include "MooseSystem.h"

#include "libmesh_common.h"

#include <vector>
#include <map>

#include "mesh.h"
#include "vector_value.h"

class PenetrationLocator
{
public:

  PenetrationLocator(MooseSystem & moose_system, Mesh & mesh, std::vector<unsigned int> master, short int slave);
  void detectPenetration();

  Real penetrationDistance(unsigned int node_id) const;
  RealVectorValue penetrationNorm(unsigned int node_id) const;
  
private:
  /**
   * Data structure used to hold penetation information
   */
  class PenetrationInfo 
  {
  public:
    PenetrationInfo(unsigned int elem_id, RealVectorValue norm, Real norm_distance);

    PenetrationInfo(const PenetrationInfo & p);
    
    unsigned int _elem_id;
    RealVectorValue _norm;
    Real _norm_distance;
  };

  MooseSystem & _moose_system;

  RealVectorValue normal(const Elem & side, const Point & p0);
  Real normDistance(const Elem & side, const Point & p0);

  int intersect2D_Segments( Point S1P0, Point S1P1, Point S2P0, Point S2P1, Point* I0, Point* I1 );
  int inSegment(Point P, Point SP0, Point SP1);
  
  Mesh & _mesh;
  std::vector<unsigned int> _master_boundary;
  short int _slave_boundary;

  /**
   * Data structure of nodes and their associated penetration information
   */
  std::map<unsigned int, PenetrationInfo *> _penetrated_elems;
};


#endif //PENETRATIONLOCATOR_H
