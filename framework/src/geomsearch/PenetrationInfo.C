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

#include "PenetrationLocator.h"
#include "ArbitraryQuadrature.h"
#include "LineSegment.h"
#include "NearestNodeLocator.h"
#include "MooseMesh.h"
#include "SubProblem.h"
#include "GeometricSearchData.h"
#include "PenetrationThread.h"
#include "Moose.h"


PenetrationInfo::PenetrationInfo(const Node * node, const Elem * elem, Elem * side, unsigned int side_num, RealVectorValue norm, Real norm_distance, Real tangential_distance, const Point & closest_point, const Point & closest_point_ref, const Point & closest_point_on_face_ref, std::vector<Node*> off_edge_nodes, const std::vector<std::vector<Real> > & side_phi, const std::vector<RealGradient> & dxyzdxi, const std::vector<RealGradient> & dxyzdeta, const std::vector<RealGradient> & d2xyzdxideta)
  :_node(node),
   _elem(elem),
   _side(side),
   _side_num(side_num),
   _normal(norm),
   _distance(norm_distance),
   _tangential_distance(tangential_distance),
   _closest_point(closest_point),
   _closest_point_ref(closest_point_ref),
   _closest_point_on_face_ref(closest_point_on_face_ref),
   _off_edge_nodes(off_edge_nodes),
   _side_phi(side_phi),
   _dxyzdxi(dxyzdxi),
   _dxyzdeta(dxyzdeta),
   _d2xyzdxideta(d2xyzdxideta),
   _accumulated_slip(0.0),
   _accumulated_slip_old(0.0),
   _frictional_energy(0.0),
   _frictional_energy_old(0.0),
   _update(true),
   _penetrated_at_beginning_of_step(false),
   _mech_status(MS_NO_CONTACT)
{}


PenetrationInfo::PenetrationInfo(const PenetrationInfo & p) :
    _node(p._node),
    _elem(p._elem),
    _side(p._side), // Which one now owns _side?  There will be trouble if (when)
                    // both delete _side
    _side_num(p._side_num),
    _normal(p._normal),
    _distance(p._distance),
    _tangential_distance(p._tangential_distance),
    _closest_point(p._closest_point),
    _closest_point_ref(p._closest_point_ref),
    _closest_point_on_face_ref(p._closest_point_on_face_ref),
    _off_edge_nodes(p._off_edge_nodes),
    _side_phi(p._side_phi),
    _dxyzdxi(p._dxyzdxi),
    _dxyzdeta(p._dxyzdeta),
    _d2xyzdxideta(p._d2xyzdxideta),
    _starting_elem(p._starting_elem),
    _starting_side_num(p._starting_side_num),
    _starting_closest_point_ref(p._starting_closest_point_ref),
    _incremental_slip(p._incremental_slip),
    _accumulated_slip(p._accumulated_slip),
    _accumulated_slip_old(p._accumulated_slip_old),
    _frictional_energy(p._frictional_energy),
    _frictional_energy_old(p._frictional_energy_old),
    _contact_force(p._contact_force),
    _contact_force_old(p._contact_force_old),
    _update(p._update),
    _penetrated_at_beginning_of_step(p._penetrated_at_beginning_of_step),
    _mech_status(p._mech_status)
{}

PenetrationInfo::~PenetrationInfo()
{
  delete _side;
}

