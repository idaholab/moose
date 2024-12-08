//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PenetrationInfo.h"
#include "ArbitraryQuadrature.h"
#include "LineSegment.h"
#include "NearestNodeLocator.h"
#include "MooseMesh.h"
#include "SubProblem.h"
#include "GeometricSearchData.h"
#include "PenetrationThread.h"
#include "Moose.h"
#include "MooseMesh.h"

#include "libmesh/elem.h"

using namespace libMesh;

PenetrationInfo::PenetrationInfo(const Elem * elem,
                                 const Elem * side,
                                 unsigned int side_num,
                                 RealVectorValue norm,
                                 Real norm_distance,
                                 Real tangential_distance,
                                 const Point & closest_point,
                                 const Point & closest_point_ref,
                                 const Point & closest_point_on_face_ref,
                                 std::vector<const Node *> off_edge_nodes,
                                 const std::vector<std::vector<Real>> & side_phi,
                                 const std::vector<std::vector<RealGradient>> & side_grad_phi,
                                 const std::vector<RealGradient> & dxyzdxi,
                                 const std::vector<RealGradient> & dxyzdeta,
                                 const std::vector<RealGradient> & d2xyzdxideta)
  : _elem(elem),
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
    _side_grad_phi(side_grad_phi),
    _dxyzdxi(dxyzdxi),
    _dxyzdeta(dxyzdeta),
    _d2xyzdxideta(d2xyzdxideta),
    _starting_elem(NULL),
    _starting_side_num(libMesh::invalid_uint),
    _starting_closest_point_ref(0),
    _incremental_slip(0),
    _accumulated_slip(0.0),
    _accumulated_slip_old(0.0),
    _frictional_energy(0.0),
    _frictional_energy_old(0.0),
    _contact_force(0),
    _contact_force_old(0),
    _lagrange_multiplier(0),
    _lagrange_multiplier_slip(0),
    _locked_this_step(0),
    _mech_status(MS_NO_CONTACT),
    _mech_status_old(MS_NO_CONTACT),
    _incremental_slip_prev_iter(0),
    _slip_reversed(false),
    _slip_tol(0)
{
}

/*
PenetrationInfo::PenetrationInfo(const PenetrationInfo & p)
  : _elem(p._elem),
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
    _side_grad_phi(p._side_grad_phi),
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
    _lagrange_multiplier(p._lagrange_multiplier),
    _lagrange_multiplier_slip(p._lagrange_multiplier_slip),

    _locked_this_step(p._locked_this_step),
    _mech_status(p._mech_status),
    _mech_status_old(p._mech_status_old),
    _incremental_slip_prev_iter(p._incremental_slip_prev_iter),
    _slip_reversed(p._slip_reversed),
    _slip_tol(p._slip_tol)
{
}
*/

PenetrationInfo::PenetrationInfo()
  : _elem(NULL),
    _side(NULL),
    _side_num(0),
    _normal(0),
    _distance(0),
    _tangential_distance(0),
    _closest_point(0),
    _closest_point_ref(0),
    _closest_point_on_face_ref(0),
    _off_edge_nodes(0),
    _side_phi(0),
    _side_grad_phi(0),
    _dxyzdxi(0),
    _dxyzdeta(0),
    _d2xyzdxideta(0),
    _starting_elem(NULL),
    _starting_side_num(libMesh::invalid_uint),
    _starting_closest_point_ref(0),
    _incremental_slip(0),
    _accumulated_slip(0.0),
    _accumulated_slip_old(0.0),
    _frictional_energy(0.0),
    _frictional_energy_old(0.0),
    _contact_force(0),
    _contact_force_old(0),
    _lagrange_multiplier(0),
    _lagrange_multiplier_slip(0),
    _locked_this_step(0),
    _mech_status(MS_NO_CONTACT),
    _mech_status_old(MS_NO_CONTACT),
    _incremental_slip_prev_iter(0),
    _slip_reversed(false),
    _slip_tol(0)
{
}

PenetrationInfo::~PenetrationInfo() { delete _side; }

template <>
void
dataStore(std::ostream & stream, PenetrationInfo *& pinfo, void * context)
{
  if (!context)
    mooseError("Can only store PenetrationInfo objects using a MooseMesh context!");

  if (pinfo)
  {
    // Store 1 so that we know that this pinfo really exists!
    unsigned int i = 1;
    storeHelper(stream, i, context);

    storeHelper(stream, pinfo->_elem, context);
    // Not storing the side element as we will need to recreate it on load
    storeHelper(stream, pinfo->_side_num, context);
    storeHelper(stream, pinfo->_normal, context);
    storeHelper(stream, pinfo->_distance, context);
    storeHelper(stream, pinfo->_tangential_distance, context);
    storeHelper(stream, pinfo->_closest_point, context);
    storeHelper(stream, pinfo->_closest_point_ref, context);
    storeHelper(stream, pinfo->_closest_point_on_face_ref, context);
    storeHelper(stream, pinfo->_off_edge_nodes, context);
    storeHelper(stream, pinfo->_side_phi, context);
    storeHelper(stream, pinfo->_side_grad_phi, context);
    storeHelper(stream, pinfo->_dxyzdxi, context);
    storeHelper(stream, pinfo->_dxyzdeta, context);
    storeHelper(stream, pinfo->_d2xyzdxideta, context);
    storeHelper(stream, pinfo->_starting_elem, context);
    storeHelper(stream, pinfo->_starting_side_num, context);
    storeHelper(stream, pinfo->_starting_closest_point_ref, context);
    storeHelper(stream, pinfo->_incremental_slip, context);
    storeHelper(stream, pinfo->_accumulated_slip, context);
    storeHelper(stream, pinfo->_frictional_energy, context);
    storeHelper(stream, pinfo->_contact_force, context);
    storeHelper(stream, pinfo->_lagrange_multiplier, context);
    storeHelper(stream, pinfo->_lagrange_multiplier_slip, context);
    storeHelper(stream, pinfo->_mech_status, context);
    storeHelper(stream, pinfo->_mech_status_old, context);

    // Don't need frictional_energy_old, accumulated_slip_old, contact_force_old, or
    // locked_this_step
    // because they are always set by the constraints at the beginning of a new time step.
  }
  else
  {
    // Store 0 so that we know that this pinfo is NULL
    unsigned int i = 0;
    storeHelper(stream, i, context);
  }
}

template <>
void
dataLoad(std::istream & stream, PenetrationInfo *& pinfo, void * context)
{
  if (!context)
    mooseError("Can only load PenetrationInfo objects using a MooseMesh context!");

  // First, see if this is supposed to be NULL
  unsigned int i = 0;
  loadHelper(stream, i, context);
  if (i)
  {
    pinfo = new PenetrationInfo();

    loadHelper(stream, pinfo->_elem, context);
    loadHelper(stream, pinfo->_side_num, context);
    // Rebuild the side element.
    pinfo->_side = pinfo->_elem->build_side_ptr(pinfo->_side_num, false).release();

    loadHelper(stream, pinfo->_normal, context);
    loadHelper(stream, pinfo->_distance, context);
    loadHelper(stream, pinfo->_tangential_distance, context);
    loadHelper(stream, pinfo->_closest_point, context);
    loadHelper(stream, pinfo->_closest_point_ref, context);
    loadHelper(stream, pinfo->_closest_point_on_face_ref, context);
    loadHelper(stream, pinfo->_off_edge_nodes, context);
    loadHelper(stream, pinfo->_side_phi, context);
    loadHelper(stream, pinfo->_side_grad_phi, context);
    loadHelper(stream, pinfo->_dxyzdxi, context);
    loadHelper(stream, pinfo->_dxyzdeta, context);
    loadHelper(stream, pinfo->_d2xyzdxideta, context);
    loadHelper(stream, pinfo->_starting_elem, context);
    loadHelper(stream, pinfo->_starting_side_num, context);
    loadHelper(stream, pinfo->_starting_closest_point_ref, context);
    loadHelper(stream, pinfo->_incremental_slip, context);
    loadHelper(stream, pinfo->_accumulated_slip, context);
    loadHelper(stream, pinfo->_frictional_energy, context);
    loadHelper(stream, pinfo->_contact_force, context);
    loadHelper(stream, pinfo->_lagrange_multiplier, context);
    loadHelper(stream, pinfo->_lagrange_multiplier_slip, context);
    loadHelper(stream, pinfo->_mech_status, context);
    loadHelper(stream, pinfo->_mech_status_old, context);

    // Don't need frictional_energy_old, accumulated_slip_old, contact_force_old, or
    // locked_this_step
    // because they are always set by the constraints at the beginning of a new time step.
  }
  else
    pinfo = NULL;
}
