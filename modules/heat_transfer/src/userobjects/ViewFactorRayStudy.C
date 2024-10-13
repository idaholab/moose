//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ViewFactorRayStudy.h"

// Local includes
#include "ViewFactorRayBC.h"
#include "GeometryUtils.h"

// libMesh includes
#include "libmesh/parallel_algebra.h"
#include "libmesh/parallel_sync.h"
#include "libmesh/enum_quadrature_type.h"
#include "libmesh/fe_base.h"
#include "libmesh/quadrature.h"

// Ray tracing includes
#include "ReflectRayBC.h"
#include "RayTracingPackingUtils.h"

using namespace libMesh;

registerMooseObject("HeatTransferApp", ViewFactorRayStudy);

InputParameters
ViewFactorRayStudy::validParams()
{
  auto params = RayTracingStudy::validParams();

  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "The list of boundaries where view factors are desired");

  MooseEnum qorders("CONSTANT FIRST SECOND THIRD FOURTH FIFTH SIXTH SEVENTH EIGHTH NINTH TENTH "
                    "ELEVENTH TWELFTH THIRTEENTH FOURTEENTH FIFTEENTH SIXTEENTH SEVENTEENTH "
                    "EIGHTTEENTH NINTEENTH TWENTIETH",
                    "CONSTANT");
  params.addParam<MooseEnum>("face_order", qorders, "The face quadrature rule order");

  MooseEnum qtypes("GAUSS GRID", "GRID");
  params.addParam<MooseEnum>("face_type", qtypes, "The face quadrature type");

  MooseEnum convention("positive=0 negative=1", "positive");
  params.addParam<MooseEnum>(
      "internal_convention",
      convention,
      "The convention for spawning rays from internal sidesets; denotes the sign of the dot "
      "product between a ray and the internal sideset side normal");

  params.addParam<unsigned int>(
      "polar_quad_order",
      16,
      "Order of the polar quadrature [polar angle is between ray and normal]. Must be even.");
  params.addParam<unsigned int>(
      "azimuthal_quad_order",
      8,
      "Order of the azimuthal quadrature per quadrant [azimuthal angle is measured in "
      "a plane perpendicular to the normal].");

  // Shouldn't ever need RayKernels for view factors
  params.set<bool>("ray_kernel_coverage_check") = false;
  params.suppressParameter<bool>("ray_kernel_coverage_check");

  // So that the study executes before the RayTracingViewFactor
  params.set<bool>("force_preaux") = true;
  params.suppressParameter<bool>("force_preaux");

  // Need to use internal sidesets
  params.set<bool>("use_internal_sidesets") = true;
  params.suppressParameter<bool>("use_internal_sidesets");

  // Don't verify Rays in opt mode by default - it's expensive
  params.set<bool>("verify_rays") = false;

  // No need to use Ray registration
  params.set<bool>("_use_ray_registration") = false;
  // Do not need to bank Rays on completion
  params.set<bool>("_bank_rays_on_completion") = false;

  params.addClassDescription(
      "This ray study is used to compute view factors in cavities with obstruction. It sends out "
      "rays from surfaces bounding the radiation cavity into a set of directions determined by an "
      "angular quadrature. The rays are tracked and view factors are computed by determining the "
      "surface where the ray dies.");
  return params;
}

ViewFactorRayStudy::ViewFactorRayStudy(const InputParameters & parameters)
  : RayTracingStudy(parameters),
    _bnd_ids_vec(_mesh.getBoundaryIDs(getParam<std::vector<BoundaryName>>("boundary"))),
    _bnd_ids(_bnd_ids_vec.begin(), _bnd_ids_vec.end()),
    _internal_convention(getParam<MooseEnum>("internal_convention")),
    _ray_index_start_bnd_id(registerRayAuxData("start_bnd_id")),
    _ray_index_start_total_weight(registerRayAuxData("start_total_weight")),
    _fe_face(FEBase::build(_mesh.dimension(), FEType(CONSTANT, MONOMIAL))),
    _q_face(QBase::build(Moose::stringToEnum<QuadratureType>(getParam<MooseEnum>("face_type")),
                         _mesh.dimension() - 1,
                         Moose::stringToEnum<Order>(getParam<MooseEnum>("face_order")))),
    _is_3d(_mesh.dimension() == 3),
    _threaded_vf_info(libMesh::n_threads())
{
  _fe_face->attach_quadrature_rule(_q_face.get());
  _fe_face->get_xyz();

  // create angular quadrature
  if (!_is_3d)
  {
    // In 2D, we integrate over angle theta instead of mu = cos(theta)
    // The integral over theta is approximated using a Gauss Legendre
    // quadrature. The integral we need to approximate is given by:
    //
    // int_{-pi/2}^{pi/2} cos(theta) d theta
    //
    // We get abscissae x and weight w for range of integration
    // from 0 to 1 and then rescale it to the integration range
    //
    std::vector<Real> x;
    std::vector<Real> w;
    RayTracingAngularQuadrature::gaussLegendre(
        2 * getParam<unsigned int>("polar_quad_order"), x, w);

    _2d_aq_angles.resize(x.size());
    _2d_aq_weights.resize(x.size());
    for (unsigned int j = 0; j < x.size(); ++j)
    {
      _2d_aq_angles[j] = (2 * x[j] - 1) * M_PI / 2;
      _2d_aq_weights[j] = w[j] * M_PI;
    }
    _num_dir = _2d_aq_angles.size();
  }
  else
  {
    _3d_aq = std::make_unique<RayTracingAngularQuadrature>(
        _mesh.dimension(),
        getParam<unsigned int>("polar_quad_order"),
        4 * getParam<unsigned int>("azimuthal_quad_order"),
        /* mu_min = */ 0,
        /* mu_max = */ 1);

    _num_dir = _3d_aq->numDirections();
  }
}

void
ViewFactorRayStudy::initialSetup()
{
  RayTracingStudy::initialSetup();

  // We optimized away RayKernels, so don't allow them
  if (hasRayKernels(/* tid = */ 0))
    mooseError("Not compatible with RayKernels.");

  // RayBC coverage checks (at least one ViewFactorRayBC and optionally a ReflectRayBC
  // on ONLY external boundaries).
  std::vector<RayBoundaryConditionBase *> ray_bcs;
  RayTracingStudy::getRayBCs(ray_bcs, 0);
  unsigned int vf_bc_count = 0;
  for (RayBoundaryConditionBase * rbc : ray_bcs)
  {
    auto view_factor_bc = dynamic_cast<ViewFactorRayBC *>(rbc);
    if (view_factor_bc)
    {
      ++vf_bc_count;

      if (!view_factor_bc->hasBoundary(_bnd_ids))
        mooseError("The boundary restriction of ",
                   rbc->type(),
                   " '",
                   rbc->name(),
                   "' does not match 'boundary'");
    }
    else
    {
      auto reflect_bc = dynamic_cast<ReflectRayBC *>(rbc);
      if (reflect_bc)
      {
        if (reflect_bc->hasBoundary(_bnd_ids))
          mooseError("The boundaries applied in ReflectRayBC '",
                     rbc->name(),
                     "' cannot include any of the boundaries in ",
                     type());

        for (const BoundaryID internal_bnd_id : getInternalSidesets())
          if (reflect_bc->hasBoundary(internal_bnd_id))
            mooseError("The ReflectRayBC '",
                       rbc->name(),
                       "' is defined on an internal boundary (",
                       internal_bnd_id,
                       ").\n\n",
                       "This is not allowed for view factor computation.");
      }
      else
        mooseError("Does not support the ",
                   rbc->type(),
                   " ray boundary condition.\nSupported RayBCs: ReflectRayBC and ViewFactorRayBC.");
    }
    if (vf_bc_count != 1)
      mooseError("Requires one and only one ViewFactorRayBC.");
  }
}

void
ViewFactorRayStudy::preExecuteStudy()
{
  // Clear and zero the view factor maps we're about to accumulate into for each thread
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    _threaded_vf_info[tid].clear();
    for (const BoundaryID from_id : _bnd_ids)
      for (const BoundaryID to_id : _bnd_ids)
        _threaded_vf_info[tid][from_id][to_id] = 0;
  }

  generateStartElems();
}

void
ViewFactorRayStudy::postExecuteStudy()
{
  // Finalize the cumulative _vf_info;
  _vf_info.clear();
  for (const BoundaryID from_id : _bnd_ids)
    for (const BoundaryID to_id : _bnd_ids)
    {
      Real & entry = _vf_info[from_id][to_id];

      // Zero before summing
      entry = 0;

      // Sum over threads
      for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
        entry += _threaded_vf_info[tid][from_id][to_id];

      // Sum over processors
      _communicator.sum(entry);
    }
}

void
ViewFactorRayStudy::generateRays()
{
  TIME_SECTION("generateRays", 3, "ViewFactorRayStudy Generating Rays");

  // Determine number of Rays and points to allocate space before generation and for output
  std::size_t num_local_rays = 0;
  std::size_t num_local_start_points = 0;
  for (const auto & start_elem : _start_elems)
  {
    num_local_start_points += start_elem._points.size();
    num_local_rays += start_elem._points.size() * _num_dir;
  }

  // Print out totals while we're here
  std::size_t num_total_points = num_local_start_points;
  std::size_t num_total_rays = num_local_rays;
  _communicator.sum(num_total_points);
  _communicator.sum(num_total_rays);
  _console << "ViewFactorRayStudy generated " << num_total_points
           << " points with an angular quadrature of " << _num_dir
           << " directions per point requiring " << num_total_rays << " rays" << std::endl;

  // Reserve space in the buffer ahead of time before we fill it
  reserveRayBuffer(num_local_rays);

  Point direction;
  unsigned int num_rays_skipped = 0;

  // loop through all starting points and spawn rays from each for each point and angle
  for (const auto & start_elem : _start_elems)
  {
    // Get normal for the element we're starting on
    auto inward_normal =
        getSideNormal(start_elem._start_elem, start_elem._incoming_side, /* tid = */ 0);
    // We actually want the normal of the original element (remember that we may swap starting
    // elements to the element on the other face per requirements of the ray tracer)
    if (start_elem._start_elem != start_elem._elem)
      inward_normal *= -1;
    // Lastly, if the boundary is external and the internal convention is positive, we must
    // switch the normal because our AQ uses the inward normal
    if (_internal_convention == 0 &&
        !start_elem._start_elem->neighbor_ptr(start_elem._incoming_side))
      inward_normal *= -1;

    // Rotation for the quadrature to align with the normal; in 3D we can do all of this
    // once up front using the 3D aq object. For 2D, we will do it within the direction loop
    if (_is_3d)
      _3d_aq->rotate(inward_normal);

    // Loop through all points and then all directions
    for (std::size_t start_i = 0; start_i < start_elem._points.size(); ++start_i)
      for (std::size_t l = 0; l < _num_dir; ++l)
      {
        // Get direction of the ray; in 3D we already rotated, in 2D we rotate here
        if (_is_3d)
          direction = _3d_aq->getDirection(l);
        else
        {
          const Real sin_theta = std::sin(_2d_aq_angles[l]);
          const Real cos_theta = std::cos(_2d_aq_angles[l]);
          direction(0) = cos_theta * inward_normal(0) - sin_theta * inward_normal(1);
          direction(1) = sin_theta * inward_normal(0) + cos_theta * inward_normal(1);
          direction(2) = 0;
        }

        // Angular weight function differs in 2D/3D
        // 2D: the quadrature abscissae are the angles between direction & normal.
        //     The integrand is the cosine of that angle
        // 3D: the quadrature abscissae are the azimuthal angle phi and the cosine of the angle
        //     between normal and direction (= mu). The integrand is mu in that case.
        const auto awf = _is_3d ? inward_normal * direction * _3d_aq->getTotalWeight(l)
                                : std::cos(_2d_aq_angles[l]) * _2d_aq_weights[l];
        const auto start_weight = start_elem._weights[start_i] * awf;

        // Skip the ray if it exists the domain through the non-planar side it is starting from.
        // We do not expect there are any neighbor elements to track it on if it exits the
        // non-planar side.
        bool intersection_found = false;
        if (_is_3d && start_elem._start_elem &&
            !start_elem._start_elem->neighbor_ptr(start_elem._incoming_side) &&
            sideIsNonPlanar(start_elem._start_elem, start_elem._incoming_side))
        {
          // Find edge on side that is 'in front' of the future ray
          Point intersection_point(std::numeric_limits<Real>::max(), -1, -1);
          const auto side_elem = start_elem._start_elem->side_ptr(start_elem._incoming_side);
          Point proj_dir;
          for (const auto edge_i : side_elem->side_index_range())
          {
            const auto edge_1 = side_elem->side_ptr(edge_i);
            // Project direction onto (start_point, node 1, node 2)
            const auto d1 = *edge_1->node_ptr(0) - start_elem._points[start_i];
            const auto d2 = *edge_1->node_ptr(1) - start_elem._points[start_i];
            const auto d1_unit = d1.unit();
            const auto d2_unit = d2.unit();
            // If the starting point is aligned with the edge, it wont cross it
            if (MooseUtils::absoluteFuzzyEqual(std::abs(d1_unit * d2_unit), 1))
              continue;
            const auto normal = (d1_unit.cross(d2_unit)).unit();

            // One of the nodes must be in front of the start point following the direction
            if (d1 * direction < 0 && d2 * direction < 0)
              continue;

            proj_dir = (direction - (direction * normal) * normal).unit();

            // Only the side of interest will have the projected direction in between d1 and d2
            if ((proj_dir * d2_unit > d1_unit * d2_unit) &&
                (proj_dir * d1_unit > d1_unit * d2_unit))
            {
              const auto dist = geom_utils::distanceFromLine(
                  start_elem._points[start_i], *edge_1->node_ptr(0), *edge_1->node_ptr(1));
              // Ortho-normalize the base on the plane
              intersection_point = start_elem._points[start_i] + dist * proj_dir;
              intersection_found = true;
              break;
            }
          }

          // Skip the ray if it goes out of the element
          const auto grazing_dir = (intersection_point - start_elem._points[start_i]).unit();
          if (intersection_found && inward_normal * direction < inward_normal * grazing_dir)
          {
            num_rays_skipped++;
            continue;
          }
        }

        // Acquire a Ray and fill with the starting information
        std::shared_ptr<Ray> ray = acquireRay();
        ray->setStart(
            start_elem._points[start_i], start_elem._start_elem, start_elem._incoming_side);
        ray->setStartingDirection(direction);
        ray->auxData(_ray_index_start_bnd_id) = start_elem._bnd_id;
        ray->auxData(_ray_index_start_total_weight) = start_weight;

        // Move the Ray into the buffer to be traced
        moveRayToBuffer(ray);
      }
  }
  if (num_rays_skipped)
    mooseInfo(num_rays_skipped,
              " rays were skipped as they exited the mesh at their starting point through "
              "non-planar sides.");
}

void
ViewFactorRayStudy::addToViewFactorInfo(Real value,
                                        const BoundaryID from_id,
                                        const BoundaryID to_id,
                                        const THREAD_ID tid)
{
  mooseAssert(currentlyPropagating(), "Can only be called during Ray tracing");
  mooseAssert(_threaded_vf_info[tid].count(from_id),
              "Threaded view factor info does not have from boundary");
  mooseAssert(_threaded_vf_info[tid][from_id].count(to_id),
              "Threaded view factor info does not have from -> to boundary");

  _threaded_vf_info[tid][from_id][to_id] += value;
}

Real
ViewFactorRayStudy::viewFactorInfo(const BoundaryID from_id, const BoundaryID to_id) const
{
  auto it = _vf_info.find(from_id);
  if (it == _vf_info.end())
    mooseError("From boundary id ", from_id, " not in view factor map.");

  auto itt = it->second.find(to_id);
  if (itt == it->second.end())
    mooseError("From boundary id ", from_id, " to boundary_id ", to_id, " not in view factor map.");
  return itt->second;
}

void
ViewFactorRayStudy::generateStartElems()
{
  const auto & points = _fe_face->get_xyz();
  const auto & weights = _fe_face->get_JxW();

  // Clear before filling
  _start_elems.clear();

  // Starting elements we have that are on the wrong side of an internal boundary
  std::unordered_map<processor_id_type, std::vector<StartElem>> send_start_map;

  // Get all possible points on the user defined boundaries on this proc
  for (const BndElement * belem : *_mesh.getBoundaryElementRange())
  {
    const Elem * elem = belem->_elem;
    const auto side = belem->_side;
    const auto bnd_id = belem->_bnd_id;

    // Skip if we don't own you
    if (elem->processor_id() != _pid)
      continue;

    // Skip if the boundary id isn't one we're looking for
    if (!_bnd_ids.count(bnd_id))
      continue;

    // Sanity check on QGRID not working on some types
    if (_q_face->type() == libMesh::QGRID && elem->type() == TET4)
      mooseError(
          "Cannot use GRID quadrature type with tetrahedral elements in ViewFactorRayStudy '",
          _name,
          "'");

    // The elem/side that we will actually start the trace from
    // (this may change on internal sidesets)
    const Elem * start_elem = elem;
    auto start_side = side;

    // Reinit this face for points
    _fe_face->reinit(elem, side);

    // See if this boundary is internal
    const Elem * neighbor = elem->neighbor_ptr(side);
    if (neighbor)
    {
      if (!neighbor->active())
        mooseError(type(), " does not work with adaptivity");

      // With the positive convention, the Rays that we want to spawn from internal boundaries
      // have positive dot products with the outward normal on the side. The ray-tracer requires
      // that we provide an element incoming side that is actually incoming (the dot product with
      // the direction and the normal is negative). Therefore, switch the physical trace to start
      // from the other element and the corresponding side
      if (_internal_convention == 0)
      {
        start_elem = neighbor;
        start_side = neighbor->which_neighbor_am_i(elem);
      }
    }

    // If we own the true starting elem, add to our start info. Otherwise, package the
    // start info to be sent to the processor that will actually start this trace
    const auto start_pid = start_elem->processor_id();
    auto & add_to = _pid ? _start_elems : send_start_map[start_pid];
    add_to.emplace_back(elem, start_elem, start_side, bnd_id, points, weights);
  }

  // If the internal convention is positive, we may have points that we switched to another
  // element for the actual trace, so communicate those to the processors that will
  // actually be starting them
  if (_internal_convention == 0)
  {
    // Functor that takes in StartElems and appends them to our local list
    auto append_start_elems = [this](processor_id_type, const std::vector<StartElem> & start_elems)
    {
      _start_elems.reserve(_start_elems.size() + start_elems.size());
      for (const StartElem & start_elem : start_elems)
        _start_elems.emplace_back(start_elem);
    };

    // Communicate and act on data
    Parallel::push_parallel_packed_range(_communicator, send_start_map, this, append_start_elems);
  }
}

namespace libMesh
{
namespace Parallel
{

unsigned int
Packing<ViewFactorRayStudy::StartElem>::packing_size(const std::size_t num_points)
{
  // Number of points, elem_id, start_elem_id, incoming_side, bnd_id
  unsigned int total_size = 5;
  // Points
  total_size += num_points * 3;
  // Weights
  total_size += num_points;

  return total_size;
}

unsigned int
Packing<ViewFactorRayStudy::StartElem>::packed_size(typename std::vector<Real>::const_iterator in)
{
  const std::size_t num_points = *in++;
  return packing_size(num_points);
}

unsigned int
Packing<ViewFactorRayStudy::StartElem>::packable_size(
    const ViewFactorRayStudy::StartElem & start_elem, const void *)
{
  mooseAssert(start_elem._points.size() == start_elem._weights.size(), "Size mismatch");
  return packing_size(start_elem._points.size());
}

template <>
ViewFactorRayStudy::StartElem
Packing<ViewFactorRayStudy::StartElem>::unpack(std::vector<Real>::const_iterator in,
                                               ViewFactorRayStudy * study)
{
  // StartElem to fill into
  ViewFactorRayStudy::StartElem start_elem;

  // Number of points
  const std::size_t num_points = static_cast<std::size_t>(*in++);

  // Elem id
  RayTracingPackingUtils::unpack(start_elem._elem, *in++, &study->meshBase());

  // Start elem id
  RayTracingPackingUtils::unpack(start_elem._start_elem, *in++, &study->meshBase());

  // Incoming side
  start_elem._incoming_side = static_cast<unsigned short>(*in++);

  // Boundary ID
  start_elem._bnd_id = static_cast<BoundaryID>(*in++);

  // Points
  start_elem._points.resize(num_points);
  for (std::size_t i = 0; i < num_points; ++i)
  {
    start_elem._points[i](0) = *in++;
    start_elem._points[i](1) = *in++;
    start_elem._points[i](2) = *in++;
  }

  // Weights
  start_elem._weights.resize(num_points);
  for (std::size_t i = 0; i < num_points; ++i)
    start_elem._weights[i] = *in++;

  return start_elem;
}

template <>
void
Packing<ViewFactorRayStudy::StartElem>::pack(const ViewFactorRayStudy::StartElem & start_elem,
                                             std::back_insert_iterator<std::vector<Real>> data_out,
                                             const ViewFactorRayStudy * study)
{
  // Number of points
  data_out = static_cast<buffer_type>(start_elem._points.size());

  // Elem id
  data_out = RayTracingPackingUtils::pack<buffer_type>(start_elem._elem, &study->meshBase());

  // Start elem id
  data_out = RayTracingPackingUtils::pack<buffer_type>(start_elem._start_elem, &study->meshBase());

  // Incoming side
  data_out = static_cast<buffer_type>(start_elem._incoming_side);

  // Boundary id
  data_out = static_cast<buffer_type>(start_elem._bnd_id);

  // Points
  for (const auto & point : start_elem._points)
  {
    data_out = point(0);
    data_out = point(1);
    data_out = point(2);
  }

  // Weights
  std::copy(start_elem._weights.begin(), start_elem._weights.end(), data_out);
}

} // namespace Parallel

} // namespace libMesh
