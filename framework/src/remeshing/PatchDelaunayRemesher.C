//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PatchDelaunayRemesher.h"

#include "ElementQualityChecker.h"
#include "FEProblemBase.h"
#include "MeshTriangulationUtils.h"
#include "MooseMesh.h"
#include "MooseMeshUtils.h"
#include "MooseVariableFieldBase.h"

#include "libmesh/boundary_info.h"
#include "libmesh/elem.h"
#include "libmesh/elem_quality.h"
#include "libmesh/enum_elem_type.h"
#include "libmesh/function_base.h"
#include "libmesh/int_range.h"
#include "libmesh/mesh_base.h"
#include "libmesh/node.h"
#include "libmesh/remote_elem.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/utility.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <optional>
#include <set>
#include <unordered_map>

using namespace libMesh;

namespace
{
/**
 * Geometric tolerance, as a fraction of the mean length of the cavity boundary edges. It matches
 * the triangulated boundary vertices back onto the nodes they were built from, and it decides that
 * two boundary edges touch. The triangulator copies the loop points through unchanged, so this
 * only has to absorb round-off, and it stays far below the shortest boundary edge that is
 * accepted.
 */
const Real relative_geometric_tolerance = 1e-8;

/**
 * A boundary edge shorter than this fraction of the mean boundary edge has collapsed in the
 * deformed configuration. Two orders of magnitude above relative_geometric_tolerance, so that an
 * edge which passes this test is still much longer than the tolerance that matches its nodes.
 */
const Real collapsed_edge_fraction = 1e-6;

/**
 * Deviation of a boundary node from the straight line between its two boundary neighbors, as a
 * fraction of the distance between those neighbors, below which the three are treated as
 * collinear and the node may be dropped by the boundary coarsening. Far above round-off, so a
 * node a straight boundary placed by an earlier solve still counts, and far below the sagitta a
 * genuinely curved boundary polyline produces at any resolution, so a curved boundary is never
 * flattened.
 */
const Real collinear_deviation_fraction = 1e-6;

/**
 * Settings of the inverse distance interpolation the triangulator grades its target area with. The
 * values are the ones SurfaceDelaunayGeneratorBase registers as the defaults of
 * 'auto_area_function_num_points' and 'auto_area_function_power', so a cavity is sized the way the
 * setup time Delaunay generators size a region with the same boundary.
 */
const unsigned int auto_area_num_points = 10;
const Real auto_area_power = 1.0;

/**
 * Area of the equilateral triangle whose side has length one, which is what converts a target
 * element size to the target area the triangulator asks for.
 *
 * The exact equilateral relation is deliberate. It sizes the new triangles a little under the
 * target size, inside the dead band between coarsen_fraction times the target and the target
 * itself, where neither this remesher nor the splitting remesher selects them again. The boundary
 * derived target libMesh computes is 1.5 times this, which would size them above the length at
 * which the splitting remesher splits and hand them straight back to it.
 *
 * The dead band covers the triangles this sizes and no more. It says nothing about the ones built
 * against a pinned cavity boundary, whose edges the triangulation cannot choose, and a boundary
 * left inside over refined territory is what buildCavities grows the cavity out of before any of
 * this applies.
 */
const Real equilateral_area_per_size_squared = std::sqrt(3.0) / 4.0;

/**
 * Shortest edge of a right isosceles triangle as a fraction of its diameter, which is what turns
 * the over refinement threshold of an element into the threshold of a cavity boundary edge.
 *
 * The selection compares a diameter, Elem::hmax(), to coarsen_fraction times the target, while a
 * cavity boundary offers a single edge to measure instead. A structured TRI3 mesh is built out of
 * right isosceles triangles, and red green refinement makes children whose shortest edge is again
 * the diameter over sqrt(2), so scaling the threshold by the same factor states one predicate in
 * the two measures. An element that is not over refined then has no edge that counts as short,
 * which is what stops the growth as soon as it leaves the over refined region. Comparing an edge
 * to the unscaled bound instead makes the shortest edge of a correctly sized element look short,
 * and the growth never converges.
 */
const Real shortest_edge_per_diameter = 1.0 / std::sqrt(2.0);

/**
 * Target triangle area at a point, out of the target element size the sizing variable carries on
 * the elements of one cavity.
 *
 * The triangulator samples this at the vertices of the triangles it is deciding whether to refine.
 * Those are points rather than elements, so each sample is located in the cavity first.
 */
class CavitySizingAreaFunction : public FunctionBase<Real>
{
public:
  /**
   * @param elements the cavity elements that carry a target, in increasing id order
   * @param areas the target area of each element of \p elements
   */
  CavitySizingAreaFunction(std::vector<const Elem *> elements, std::vector<Real> areas);

  virtual Real operator()(const Point & p, const Real time) override;

  virtual void operator()(const Point & p, const Real time, DenseVector<Real> & output) override;

  virtual std::unique_ptr<FunctionBase<Real>> clone() const override;

private:
  /// The cavity elements that carry a target, in increasing id order
  const std::vector<const Elem *> _elements;

  /// The target area of each element of _elements
  const std::vector<Real> _areas;
};

CavitySizingAreaFunction::CavitySizingAreaFunction(std::vector<const Elem *> elements,
                                                   std::vector<Real> areas)
  : _elements(std::move(elements)), _areas(std::move(areas))
{
  mooseAssert(!_elements.empty(), "A cavity with no target at all is not given an area function.");
  mooseAssert(_elements.size() == _areas.size(),
              "The table holds one target area per element, so the two are built together.");

  // The table is complete as it stands, so there is nothing for init() to do, and the target does
  // not move while the cavity is being triangulated
  this->_initialized = true;
  this->_is_time_dependent = false;
}

Real
CavitySizingAreaFunction::operator()(const Point & p, const Real /*time*/)
{
  for (const auto i : index_range(_elements))
    if (_elements[i]->contains_point(p))
      return _areas[i];

  // A sample that sits on a cavity boundary edge, or just outside one, can miss every containment
  // test by round-off, so it takes the target of the element whose centroid is nearest. The table
  // is in increasing id order and the comparison is strict, which leaves the lower id winning a
  // tie. Every rank of a replicated mesh builds this table out of the same cavity and the same
  // gathered targets, so a given sample is answered the same way on all of them.
  std::size_t nearest = 0;
  Real nearest_distance = std::numeric_limits<Real>::max();
  for (const auto i : index_range(_elements))
  {
    const Real distance = (_elements[i]->vertex_average() - p).norm();
    if (distance < nearest_distance)
    {
      nearest_distance = distance;
      nearest = i;
    }
  }

  return _areas[nearest];
}

void
CavitySizingAreaFunction::operator()(const Point & p, const Real time, DenseVector<Real> & output)
{
  output.resize(1);
  output(0) = (*this)(p, time);
}

std::unique_ptr<FunctionBase<Real>>
CavitySizingAreaFunction::clone() const
{
  return std::make_unique<CavitySizingAreaFunction>(*this);
}

/// Twice the signed area of the triangle (a, b, c), projected on the XY plane
Real
signedArea2(const Point & a, const Point & b, const Point & c)
{
  return (b(0) - a(0)) * (c(1) - a(1)) - (b(1) - a(1)) * (c(0) - a(0));
}

/// Twice the signed area a closed loop of nodes encloses, projected on the XY plane
Real
loopSignedArea2(const std::vector<Node *> & loop)
{
  Real twice_area = 0;
  for (const auto i : index_range(loop))
  {
    const Point & current = *loop[i];
    const Point & next = *loop[(i + 1) % loop.size()];
    twice_area += current(0) * next(1) - next(0) * current(1);
  }
  return twice_area;
}

/// Whether \p p lies in the XY bounding box of the segment (a0, a1), grown by \p length_tol
bool
inSegmentBox(const Point & a0, const Point & a1, const Point & p, const Real length_tol)
{
  return p(0) >= std::min(a0(0), a1(0)) - length_tol &&
         p(0) <= std::max(a0(0), a1(0)) + length_tol &&
         p(1) >= std::min(a0(1), a1(1)) - length_tol && p(1) <= std::max(a0(1), a1(1)) + length_tol;
}

/**
 * Whether the closed segments (a0, a1) and (b0, b1) have a point of the XY plane in common.
 * Segments that only touch count as intersecting: on a cavity boundary that has no pinch, two
 * edges that are not neighbors can only touch if the deformed configuration folded the boundary
 * onto itself.
 */
bool
segmentsIntersect(const Point & a0,
                  const Point & a1,
                  const Point & b0,
                  const Point & b1,
                  const Real area_tol,
                  const Real length_tol)
{
  const Real b0_of_a = signedArea2(a0, a1, b0);
  const Real b1_of_a = signedArea2(a0, a1, b1);
  const Real a0_of_b = signedArea2(b0, b1, a0);
  const Real a1_of_b = signedArea2(b0, b1, a1);

  // Each segment strictly separates the two ends of the other one
  if (((b0_of_a > area_tol && b1_of_a < -area_tol) ||
       (b0_of_a < -area_tol && b1_of_a > area_tol)) &&
      ((a0_of_b > area_tol && a1_of_b < -area_tol) || (a0_of_b < -area_tol && a1_of_b > area_tol)))
    return true;

  // An end of one segment lies on the other one
  return (std::abs(b0_of_a) <= area_tol && inSegmentBox(a0, a1, b0, length_tol)) ||
         (std::abs(b1_of_a) <= area_tol && inSegmentBox(a0, a1, b1, length_tol)) ||
         (std::abs(a0_of_b) <= area_tol && inSegmentBox(b0, b1, a0, length_tol)) ||
         (std::abs(a1_of_b) <= area_tol && inSegmentBox(b0, b1, a1, length_tol));
}
}

registerMooseObject("MooseApp", PatchDelaunayRemesher);

InputParameters
PatchDelaunayRemesher::validParams()
{
  InputParameters params = Remesher::validParams();
  params.addClassDescription(
      "Replaces patches of failing triangles with a Delaunay triangulation of the patch, pinning "
      "the nodes of the patch boundary so that sidesets and subdomain interfaces are preserved.");

  params.addParam<unsigned int>("n_layers",
                                1,
                                "Number of point-neighbor layers grown around each failing "
                                "element to form the patch that is retriangulated.");
  params.addRangeCheckedParam<Real>(
      "desired_area",
      0,
      "desired_area >= 0",
      "Target area of the new triangles, one value for the whole patch. The default of 0 grades "
      "the target off the length of each patch boundary edge instead, which is what suits a patch "
      "whose boundary spans more than one mesh size.");
  params.addParam<VariableName>(
      "sizing_variable",
      "The target element size field, a CONSTANT MONOMIAL auxiliary variable holding the element "
      "size the remesher aims for. It sizes the new triangles and is read by the over-refinement "
      "selection, so it has to be supplied together with 'coarsen_fraction'. Without it the new "
      "triangles are sized off the patch boundary edges and the quality metric selects alone.");
  params.addRangeCheckedParam<Real>(
      "coarsen_fraction",
      "coarsen_fraction > 0 & coarsen_fraction < 1",
      "An element whose diameter falls below this fraction of the value of 'sizing_variable' on it "
      "is retriangulated as over refined, on top of the elements 'quality_metric' selects. The "
      "patch is then grown until no boundary edge of it is short either, measured against this "
      "fraction divided by sqrt(2) so that the bound applies to an edge rather than to a diameter, "
      "because a pinned boundary keeps its spacing through the surgery. Requires "
      "'sizing_variable'.");

  params.addRangeCheckedParam<Real>(
      "coarsen_boundary_fraction",
      0,
      "coarsen_boundary_fraction >= 0 & coarsen_boundary_fraction < 1",
      "An external boundary node of a patch is dropped, coarsening the boundary, when it lies on "
      "the straight line between its boundary neighbors and both of the edges it joins are "
      "shorter than this fraction of the target edge length, provided the merged edge does not "
      "itself exceed the target. The default of 0 keeps every boundary node pinned. Requires a "
      "target size, from 'desired_area' or 'sizing_variable', to measure against.");

  MooseEnum quality_metric = ElementQualityChecker::QualityMetricType();
  quality_metric = "SHAPE";
  params.addParam<MooseEnum>(
      "quality_metric", quality_metric, "Quality metric that decides which triangles fail.");
  params.addParam<Real>("quality_lower_bound",
                        "Value of 'quality_metric' below which a triangle fails. Defaults to the "
                        "lower bound libMesh suggests for the metric on a triangle.");
  params.addParam<Real>("quality_upper_bound",
                        "Value of 'quality_metric' above which a triangle fails. Defaults to the "
                        "upper bound libMesh suggests for the metric on a triangle.");

  return params;
}

PatchDelaunayRemesher::PatchDelaunayRemesher(const InputParameters & parameters)
  : Remesher(parameters),
    _n_layers(getParam<unsigned int>("n_layers")),
    _desired_area(getParam<Real>("desired_area")),
    _sizing_variable(isParamValid("sizing_variable")
                         ? &_fe_problem.getVariable(/*tid=*/0,
                                                    getParam<VariableName>("sizing_variable"),
                                                    Moose::VarKindType::VAR_ANY,
                                                    Moose::VarFieldType::VAR_FIELD_STANDARD)
                         : nullptr),
    _coarsen_fraction(isParamValid("coarsen_fraction") ? getParam<Real>("coarsen_fraction") : 0),
    _coarsen_boundary_fraction(getParam<Real>("coarsen_boundary_fraction")),
    _quality_metric(getParam<MooseEnum>("quality_metric").getEnum<ElemQuality>())
{
  if (_mesh.dimension() != 2)
    mooseError(
        "This remesher triangulates in the XY plane, but the mesh is ", _mesh.dimension(), "D.");

  if (_coarsen_boundary_fraction > 0 && !isParamValid("sizing_variable") && !(_desired_area > 0))
    paramError("coarsen_boundary_fraction",
               "The boundary coarsening measures the boundary edges against a target edge length, "
               "so 'desired_area' or 'sizing_variable' has to be supplied with it.");

  if (isParamValid("coarsen_fraction") && !isParamValid("sizing_variable"))
    paramError("coarsen_fraction",
               "The over-refinement selection compares the diameter of an element to the target "
               "size field, so 'sizing_variable' has to be supplied together with "
               "'coarsen_fraction'.");

  if (isParamValid("sizing_variable") && !isParamValid("coarsen_fraction"))
    paramError("sizing_variable",
               "The target size field sizes the new triangles and drives the over-refinement "
               "selection, so 'coarsen_fraction' has to be supplied together with "
               "'sizing_variable'.");

  if (_sizing_variable)
  {
    checkElementalSizingVariable("sizing_variable", *_sizing_variable);

    if (_desired_area > 0)
      paramError("desired_area",
                 "Only one of 'desired_area' and 'sizing_variable' can set the target area of the "
                 "new triangles, and the triangulator does not support a single target area "
                 "combined with a target that varies over the patch.");
  }

  const std::vector<ElemQuality> triangle_metrics = Quality::valid(TRI3);
  if (std::find(triangle_metrics.begin(), triangle_metrics.end(), _quality_metric) ==
      triangle_metrics.end())
    paramError("quality_metric",
               "libMesh does not define this metric for TRI3 elements, which are the only "
               "elements this remesher measures.");

  const std::pair<Real, Real> suggested_bounds = Elem::build(TRI3)->qual_bounds(_quality_metric);
  _lower_bound = isParamValid("quality_lower_bound") ? getParam<Real>("quality_lower_bound")
                                                     : suggested_bounds.first;
  _upper_bound = isParamValid("quality_upper_bound") ? getParam<Real>("quality_upper_bound")
                                                     : suggested_bounds.second;
  if (_lower_bound >= _upper_bound)
    paramError("quality_lower_bound",
               "The lower bound ",
               _lower_bound,
               " of the quality metric is not below its upper bound ",
               _upper_bound,
               ", so every triangle would fail.");

  // The triangulator serializes and reduces over the communicator of the mesh it is handed, and
  // every rank triangulates every cavity redundantly, so one singleton communicator per rank turns
  // all of that into rank local work
  _communicator.split(processor_id(), processor_id(), _cavity_comm);
}

Remesher::RemeshRecord
PatchDelaunayRemesher::remesh()
{
  RemeshRecord record;

  std::vector<dof_id_type> seeds = selectFailingElements();

  unsigned int pinched = 0;
  unsigned int degenerate = 0;
  unsigned int not_triangulated = 0;
  unsigned int deferred = 0;
  std::size_t n_cavities = 0;

  std::vector<CavityPatch> patches;

  // A rejected cavity is retried grown by another point-neighbor layer: a boundary that pinches,
  // collapses, or self-touches runs through distorted elements, and growing the cavity moves the
  // boundary outward onto healthier ones while the distorted region falls inside the patch. The
  // whole set is rebuilt each round so that a grown cavity merges with any patch it now overlaps.
  constexpr unsigned int max_grow_rounds = 3;
  for (const auto round : make_range(max_grow_rounds + 1))
  {
    patches.clear();
    pinched = degenerate = not_triangulated = deferred = 0;
    std::set<dof_id_type> rejected_elements;

    const std::vector<std::vector<dof_id_type>> cavities = buildCavities(seeds, deferred);
    n_cavities = cavities.size();
    for (const auto & cavity : cavities)
    {
      std::vector<BoundaryLoop> loops;
      Real mean_edge_length = 0;
      CavityPatch patch;
      SideBoundaryIds merged_side_ids;
      if (!extractBoundaryLoops(cavity, loops))
      {
        ++pinched;
        rejected_elements.insert(cavity.begin(), cavity.end());
        continue;
      }

      thinBoundaryLoops(cavity, loops, merged_side_ids);
      if (!validateBoundaryLoops(loops, mean_edge_length))
        ++degenerate;
      else if (!triangulateCavity(cavity, loops, mean_edge_length, patch))
        ++not_triangulated;
      else
      {
        // A merged edge spans nodes that were never the two ends of one old side, so the sideset
        // it inherited is recorded on top of the ids read off the old sides
        patch.side_boundary_ids.insert(merged_side_ids.begin(), merged_side_ids.end());
        patch.orphaned_nodes = collectOrphanedNodes(cavity, loops);
        patches.push_back(std::move(patch));
        continue;
      }
      rejected_elements.insert(cavity.begin(), cavity.end());
    }

    if (rejected_elements.empty() || round == max_grow_rounds)
      break;

    // Re-seeding a rejected cavity with its own elements grows it by n_layers on the next build.
    // The accepted cavities rebuild identically from the original seeds unless a grown neighbor
    // reaches them, in which case they merge. buildCavities deduplicates the seed list.
    seeds.insert(seeds.end(), rejected_elements.begin(), rejected_elements.end());
  }

  // Rejecting a cavity leaves the mesh alone, which is indistinguishable from the criterion never
  // having fired, so the rejections are reported rather than left silent
  if (pinched + degenerate + not_triangulated)
    _console << "Remeshing: rejected " << pinched + degenerate + not_triangulated << " of "
             << n_cavities << " patches (" << pinched << " pinched, " << degenerate
             << " degenerate boundary, " << not_triangulated << " not triangulated)" << std::endl;

  // Deferring a cavity leaves the mesh alone where the criterion asked for a coarser one, which is
  // indistinguishable from the criterion never having fired, so the deferrals are reported
  if (deferred)
    _console << "Remeshing: deferred " << deferred << (deferred == 1 ? " cavity" : " cavities")
             << " whose boundary was still finer than the target size" << std::endl;

  // One node per interior vertex of a patch and one element per triangle of it
  dof_id_type n_new_nodes = 0;
  dof_id_type n_new_elements = 0;
  for (const auto & patch : patches)
  {
    n_new_nodes += cast_int<dof_id_type>(patch.new_points.size());
    n_new_elements += cast_int<dof_id_type>(patch.triangles.size());
  }

  // reserveNewEntityIds() is collective on a distributed mesh, so every rank reaches it, including
  // one that produced no patch at all
  dof_id_type next_node_id = 0;
  dof_id_type next_elem_id = 0;
  reserveNewEntityIds(n_new_nodes, n_new_elements, next_node_id, next_elem_id);

  for (const auto & patch : patches)
    splicePatch(patch, next_node_id, next_elem_id, record);

  record.changed = !record.new_elements.empty();
  return record;
}

std::vector<dof_id_type>
PatchDelaunayRemesher::selectFailingElements()
{
  const MeshBase & mesh = _mesh.getMesh();
  const bool distributed = _mesh.isDistributedMesh();

  // The ids in the table name the elements of the mesh as it stands now, and the surgery this event
  // performs replaces them, so the event opens by throwing away whatever the last one gathered
  _gathered_target_sizes.reset();

  // A rank may only replace the elements it owns, so a distributed mesh does not measure the
  // ghosted ones. A replicated mesh measures all of them on every rank, which is what makes the
  // ranks perform the same surgery. Reading the sizing field narrows the measurement to the owned
  // elements on a replicated mesh as well: current_local_solution is readable at the owned degrees
  // of freedom plus the send list, and the degrees of freedom partition by processor id even where
  // the elements do not, so a rank cannot read the target of an element it does not own. The
  // gathers below are what put the ranks back in step after that narrowing.
  const bool owned_only = distributed || _sizing_variable;

  // A cavity of a replicated mesh holds elements of every rank, because cavity growth there is not
  // restricted to the ones this rank owns, and only the owner of an element can read the target on
  // it, so the targets are gathered for every rank to size that cavity the same way. A distributed
  // mesh needs no table: its cavities hold owned elements only, so the target is legible off the
  // solution there, which is the same value by a shorter route.
  const bool gather_across_ranks = _sizing_variable && !distributed;

  std::vector<dof_id_type> failing;

  // The ids and the targets of the owned elements that carry one, as two parallel vectors because
  // that is what the communicator gathers
  std::vector<dof_id_type> target_ids;
  std::vector<Real> targets;

  for (const Elem * elem : mesh.active_element_ptr_range())
  {
    if (elem->type() != TRI3)
      continue;

    if (owned_only && elem->processor_id() != mesh.processor_id())
      continue;

    // The two selections are independent and both active: a sizing variable adds the size test to
    // the quality test rather than replacing it, and an element that fails either one is selected
    const Real metric = elem->quality(_quality_metric);
    const bool fails_quality = metric < _lower_bound || metric > _upper_bound;

    // Elem::hmax() is the largest vertex separation, which is the diameter of a straight sided
    // element and the very measure the splitting remesher compares to the same target, so the two
    // remeshers select complementary elements out of one field
    const std::optional<Real> target =
        _sizing_variable ? readTargetSize(*_sizing_variable, *elem, std::nullopt) : std::nullopt;
    const bool over_refined = target && elem->hmax() < _coarsen_fraction * *target;

    if (gather_across_ranks && target)
    {
      target_ids.push_back(elem->id());
      targets.push_back(*target);
    }

    if (fails_quality || over_refined)
      failing.push_back(elem->id());
  }

  if (gather_across_ranks)
  {
    // Every rank of a replicated mesh performs the whole surgery, so a selection narrowed to the
    // elements this rank owns needs what the other ranks measured. Every active element is owned by
    // exactly one rank, and both tests are the same function of the element whichever rank runs
    // them, so what this gathers is the set the unnarrowed loop selects. It gathers nothing on a
    // run of one rank, which owns every element, rather than being dead code there.
    _communicator.allgather(failing);

    _gathered_target_sizes = gatherTargetSizes(std::move(target_ids), std::move(targets));
  }

  // Ordered by id rather than by the order the mesh iterator produced, so that every rank of a
  // replicated mesh performs the same surgery in the same order and the copies stay identical
  std::sort(failing.begin(), failing.end());
  return failing;
}

std::optional<Real>
PatchDelaunayRemesher::targetSize(const Elem & elem) const
{
  mooseAssert(_sizing_variable,
              "The target size is only read on the paths a sizing variable turns on, so this is "
              "not reached without one.");

  // Cavity growth on a distributed mesh stops at an element of another rank, so every element a
  // cavity holds there is one this rank owns and the solution carries its target. That is the same
  // value the table would hand back, reached without gathering anything.
  if (_mesh.isDistributedMesh())
    return readTargetSize(*_sizing_variable, elem, std::nullopt);

  mooseAssert(_gathered_target_sizes,
              "selectFailingElements gathers the table at the start of the event, and it runs "
              "before any cavity is built, so reaching a cavity without one means the event sized "
              "a patch it never selected.");

  // Every element is owned by exactly one rank and every rank contributed the owned elements that
  // carry a target, so a miss is an element whose owner found the sizing variable undefined on its
  // subdomain rather than a gap in the table
  const auto it = _gathered_target_sizes->find(elem.id());
  if (it == _gathered_target_sizes->end())
    return std::nullopt;

  return it->second;
}

std::unique_ptr<FunctionBase<Real>>
PatchDelaunayRemesher::buildSizingAreaFunction(const std::vector<dof_id_type> & cavity) const
{
  mooseAssert(_sizing_variable,
              "The cavity is only sized off the field when a sizing variable was given.");
  mooseAssert(std::is_sorted(cavity.begin(), cavity.end()),
              "The nearest element fallback breaks a tie by taking the earlier element of the "
              "table, which is the lower id only while the cavity is in increasing id order.");

  const MeshBase & mesh = _mesh.getMesh();

  std::vector<const Elem *> elements;
  std::vector<Real> areas;
  for (const auto elem_id : cavity)
  {
    const Elem * elem = mesh.elem_ptr(elem_id);
    const std::optional<Real> target = targetSize(*elem);
    if (!target)
      continue;

    elements.push_back(elem);
    // The field carries a target length and the triangulator asks for a target area, which is the
    // one place the two units meet
    areas.push_back(equilateral_area_per_size_squared * *target * *target);
  }

  if (elements.empty())
    return nullptr;

  return std::make_unique<CavitySizingAreaFunction>(std::move(elements), std::move(areas));
}

std::vector<std::vector<dof_id_type>>
PatchDelaunayRemesher::buildCavities(const std::vector<dof_id_type> & seeds,
                                     unsigned int & n_deferred)
{
  MeshBase & mesh = _mesh.getMesh();
  const auto & node_to_elem = _mesh.nodeToElemMap();

  n_deferred = 0;

  // Growth stops at an element the triangulator cannot reproduce, rather than crossing it and
  // leaving a later check to reject the whole connected patch over it. On a distributed mesh it
  // stops at an element of another rank too, which is what confines each rank's surgery to the
  // elements it owns and leaves the nodes of the partition seam on a cavity boundary, pinned.
  const bool distributed = _mesh.isDistributedMesh();
  auto may_join_cavity = [&mesh, distributed](const dof_id_type elem_id)
  {
    const Elem * elem = mesh.elem_ptr(elem_id);
    return elem->type() == TRI3 && (!distributed || elem->processor_id() == mesh.processor_id());
  };

  // Growing every seed on one shared set is what merges the cavities that would have overlapped
  std::set<dof_id_type> grown(seeds.begin(), seeds.end());
  std::set<dof_id_type> frontier = grown;
  for (const auto layer : make_range(_n_layers))
  {
    libmesh_ignore(layer);
    std::set<dof_id_type> next_frontier;
    for (const auto elem_id : frontier)
    {
      const Elem * elem = mesh.elem_ptr(elem_id);
      for (const auto n : elem->node_index_range())
        for (const auto neighbor_id : libmesh_map_find(node_to_elem, elem->node_id(n)))
          if (may_join_cavity(neighbor_id) && grown.insert(neighbor_id).second)
            next_frontier.insert(neighbor_id);
    }
    frontier = std::move(next_frontier);
  }

  // Split the grown set into the pieces that are connected through element sides. Two pieces that
  // only meet at a node are retriangulated separately, which is what keeps their common node off
  // a pinched boundary.
  const auto split_into_side_connected = [&mesh](const std::set<dof_id_type> & elements)
  {
    std::vector<std::vector<dof_id_type>> components;
    std::set<dof_id_type> unassigned = elements;
    while (!unassigned.empty())
    {
      std::vector<dof_id_type> front;
      front.push_back(*unassigned.begin());
      unassigned.erase(unassigned.begin());

      std::vector<dof_id_type> component;
      while (!front.empty())
      {
        const dof_id_type elem_id = front.back();
        front.pop_back();
        component.push_back(elem_id);

        const Elem * elem = mesh.elem_ptr(elem_id);
        for (const auto side : elem->side_index_range())
        {
          // A remote neighbor is an element this rank does not hold, so it is not in the cavity
          const Elem * neighbor = elem->neighbor_ptr(side);
          if (!neighbor || neighbor == remote_elem)
            continue;

          const auto it = unassigned.find(neighbor->id());
          if (it != unassigned.end())
          {
            front.push_back(*it);
            unassigned.erase(it);
          }
        }
      }

      std::sort(component.begin(), component.end());
      components.push_back(std::move(component));
    }
    return components;
  };

  std::vector<std::vector<dof_id_type>> cavities = split_into_side_connected(grown);

  // A pinned boundary keeps its spacing through the surgery, so a cavity is only a fixed point once
  // no boundary edge of it is short against the target. The selection already put every over
  // refined element in the cavity, so this only has to cross the few elements whose edges are still
  // short, and it stops of its own accord where the mesh already meets its target.
  constexpr unsigned int max_rim_growth_rounds = 10;
  if (_sizing_variable)
    for (const auto round : make_range(max_rim_growth_rounds))
    {
      libmesh_ignore(round);
      bool grew = false;
      for (const auto & cavity : cavities)
        for (const auto & [elem_id, side] : overRefinedBoundarySides(cavity))
        {
          // The pinning that stops the n_layers point-neighbor growth above stops this growth
          // too, and the filter below defers a cavity it walled in rather than the growth reaching
          // past it
          const Elem * neighbor = mesh.elem_ptr(elem_id)->neighbor_ptr(side);
          if (neighbor && neighbor != remote_elem && may_join_cavity(neighbor->id()) &&
              grown.insert(neighbor->id()).second)
            grew = true;
        }

      if (!grew)
        break;

      cavities = split_into_side_connected(grown);
    }

  // A single side-connected cavity can still pinch, by wrapping around a node whose remaining
  // elements the growth left out. Absorbing the node's whole star moves the node into the cavity
  // interior. Healing only ever grows the set, so the loop terminates; a pinch that survives the
  // cap, because its star is blocked by a non-TRI3 element, is left for extractBoundaryLoops to
  // reject with the diagnostic.
  constexpr unsigned int max_heal_iterations = 5;
  for (const auto iteration : make_range(max_heal_iterations))
  {
    libmesh_ignore(iteration);
    bool healed = false;
    for (const auto & cavity : cavities)
      for (const auto node_id : findPinchNodes(cavity))
        for (const auto elem_id : libmesh_map_find(node_to_elem, node_id))
          if (may_join_cavity(elem_id) && grown.insert(elem_id).second)
            healed = true;
    if (!healed)
      break;
    cavities = split_into_side_connected(grown);
  }

  // Healing absorbs elements after the growth above settled, which can put a short edge back on a
  // boundary, and the growth itself stops at a side it may not cross. A cavity that still has one
  // would be retriangulated into the state that re-triggers it, so this event leaves it alone
  // instead. It is not lost: its elements are still over refined at the next event and are selected
  // again, the way a split that would reach onto another rank is deferred rather than dropped.
  if (_sizing_variable)
  {
    std::vector<std::vector<dof_id_type>> converged;
    for (auto & cavity : cavities)
      if (overRefinedBoundarySides(cavity).empty())
        converged.push_back(std::move(cavity));
      else
        ++n_deferred;

    cavities = std::move(converged);
  }

  return cavities;
}

std::vector<std::pair<dof_id_type, unsigned int>>
PatchDelaunayRemesher::overRefinedBoundarySides(const std::vector<dof_id_type> & cavity) const
{
  mooseAssert(_sizing_variable,
              "An element is only short against a target where there is a sizing variable to read "
              "the target off.");

  const MeshBase & mesh = _mesh.getMesh();
  const std::set<dof_id_type> cavity_ids(cavity.begin(), cavity.end());

  std::vector<std::pair<dof_id_type, unsigned int>> sides;
  for (const auto elem_id : cavity)
  {
    const Elem * elem = mesh.elem_ptr(elem_id);

    // The sizing variable is not defined on the subdomain of this element, which leaves its sides
    // no target to be short against
    const std::optional<Real> target = targetSize(*elem);
    if (!target)
      continue;

    for (const auto side : elem->side_index_range())
    {
      // A side is on the cavity boundary when it has no neighbor, when its neighbor is an element
      // the growth left out, or when its neighbor is an element this rank does not hold
      const Elem * neighbor = elem->neighbor_ptr(side);
      if (neighbor && neighbor != remote_elem && cavity_ids.count(neighbor->id()))
        continue;

      // A short external boundary side can never be absorbed by growing the cavity, which stops
      // at the domain boundary. With the boundary coarsening on it is thinned away instead, so it
      // does not defer the cavity; without it, the short side defers the cavity.
      if (!neighbor && _coarsen_boundary_fraction > 0)
        continue;

      const auto side_nodes = elem->nodes_on_side(side);
      const Real length = (elem->point(side_nodes[1]) - elem->point(side_nodes[0])).norm();

      // The selection test restated for a single edge, so a boundary whose element would not
      // itself be coarsen selected counts as converged. The bound is scaled from a diameter to an
      // edge; comparing an edge to the unscaled bound makes a correctly sized element look short.
      if (length < _coarsen_fraction * shortest_edge_per_diameter * *target)
        sides.emplace_back(elem_id, side);
    }
  }

  return sides;
}

std::vector<dof_id_type>
PatchDelaunayRemesher::findPinchNodes(const std::vector<dof_id_type> & cavity) const
{
  const MeshBase & mesh = _mesh.getMesh();
  const std::set<dof_id_type> cavity_ids(cavity.begin(), cavity.end());

  std::map<dof_id_type, unsigned int> outgoing;
  std::map<dof_id_type, unsigned int> incoming;
  for (const auto elem_id : cavity)
  {
    const Elem * elem = mesh.elem_ptr(elem_id);
    for (const auto side : elem->side_index_range())
    {
      const Elem * neighbor = elem->neighbor_ptr(side);
      if (neighbor && neighbor != remote_elem && cavity_ids.count(neighbor->id()))
        continue;

      const auto side_nodes = elem->nodes_on_side(side);
      ++outgoing[elem->node_id(side_nodes[0])];
      ++incoming[elem->node_id(side_nodes[1])];
    }
  }

  std::set<dof_id_type> pinch_nodes;
  for (const auto & [node_id, count] : outgoing)
    if (count > 1)
      pinch_nodes.insert(node_id);
  for (const auto & [node_id, count] : incoming)
    if (count > 1)
      pinch_nodes.insert(node_id);

  return {pinch_nodes.begin(), pinch_nodes.end()};
}

bool
PatchDelaunayRemesher::extractBoundaryLoops(const std::vector<dof_id_type> & cavity,
                                            std::vector<BoundaryLoop> & loops) const
{
  MeshBase & mesh = _mesh.getMesh();
  const std::set<dof_id_type> cavity_ids(cavity.begin(), cavity.end());

  // The boundary edges, directed the way the cavity elements are wound, so that following them
  // walks the outer boundary and the boundary of every enclosed region in opposite senses
  std::map<dof_id_type, dof_id_type> next_node;
  for (const auto elem_id : cavity)
  {
    const Elem * elem = mesh.elem_ptr(elem_id);
    for (const auto side : elem->side_index_range())
    {
      // A side is on the cavity boundary when it has no neighbor, when its neighbor is an element
      // the growth left out, or when its neighbor is an element this rank does not hold
      const Elem * neighbor = elem->neighbor_ptr(side);
      if (neighbor && neighbor != remote_elem && cavity_ids.count(neighbor->id()))
        continue;

      const auto side_nodes = elem->nodes_on_side(side);
      // A node with two outgoing boundary edges is a pinch, which leaves the loop through it
      // ambiguous. The cavity is rejected for this step rather than repaired.
      if (!next_node.emplace(elem->node_id(side_nodes[0]), elem->node_id(side_nodes[1])).second)
        return false;
    }
  }

  std::set<dof_id_type> unvisited;
  for (const auto & boundary_edge : next_node)
    unvisited.insert(boundary_edge.first);

  while (!unvisited.empty())
  {
    const dof_id_type start = *unvisited.begin();
    BoundaryLoop loop;
    dof_id_type current = start;
    do
    {
      if (!unvisited.erase(current))
        return false;
      loop.push_back(mesh.node_ptr(current));
      current = libmesh_map_find(next_node, current);
    } while (current != start);

    loops.push_back(std::move(loop));
  }

  // The outer loop encloses every other one, so it is the loop that encloses the largest area
  std::size_t outer = 0;
  Real outer_area = 0;
  for (const auto i : index_range(loops))
  {
    const Real area = std::abs(loopSignedArea2(loops[i]));
    if (area > outer_area)
    {
      outer_area = area;
      outer = i;
    }
  }
  std::swap(loops[0], loops[outer]);

  return true;
}

void
PatchDelaunayRemesher::thinBoundaryLoops(const std::vector<dof_id_type> & cavity,
                                         std::vector<BoundaryLoop> & loops,
                                         SideBoundaryIds & merged_side_ids) const
{
  if (!(_coarsen_boundary_fraction > 0))
    return;

  const MeshBase & mesh = _mesh.getMesh();
  const BoundaryInfo & boundary_info = mesh.get_boundary_info();

  /// The sideset ids and the target edge length of one external side of the cavity
  struct ExternalSide
  {
    std::vector<boundary_id_type> ids;
    Real target;
  };

  // The external sides of the cavity, keyed by their sorted node pair. Only a side with no
  // neighbor at all qualifies: a side whose neighbor this rank does not hold is a partition seam,
  // whose nodes have to stay pinned for the ranks to keep agreeing on them.
  std::map<std::pair<dof_id_type, dof_id_type>, ExternalSide> external_sides;
  std::vector<boundary_id_type> side_ids;
  for (const auto elem_id : cavity)
  {
    const Elem * elem = mesh.elem_ptr(elem_id);

    // The field carries a target length per element; a uniform desired_area is restated as the
    // edge of the right isosceles triangle of that area, the tiling a structured TRI3 mesh is
    // built out of
    const std::optional<Real> target =
        _sizing_variable ? targetSize(*elem) : std::optional<Real>(std::sqrt(2 * _desired_area));
    if (!target)
      continue;

    for (const auto side : elem->side_index_range())
    {
      if (elem->neighbor_ptr(side))
        continue;

      boundary_info.boundary_ids(elem, side, side_ids);
      // The ids of adjacent sides are compared below, which needs them in a stable order
      std::sort(side_ids.begin(), side_ids.end());
      const auto side_nodes = elem->nodes_on_side(side);
      external_sides[sortedNodePair(elem->node_id(side_nodes[0]), elem->node_id(side_nodes[1]))] = {
          side_ids, *target};
    }
  }

  if (external_sides.empty())
    return;

  for (auto & loop : loops)
  {
    // The external side behind each loop edge, where edge i runs loop[i] -> loop[i + 1], empty
    // where the edge is interior to the mesh or a partition seam
    std::vector<std::optional<ExternalSide>> edge_side(loop.size());
    for (const auto i : index_range(loop))
    {
      const auto it =
          external_sides.find(sortedNodePair(loop[i]->id(), loop[(i + 1) % loop.size()]->id()));
      if (it != external_sides.end())
        edge_side[i] = it->second;
    }

    // One node is dropped per pass, because an erase renumbers the edges behind it
    bool changed = true;
    while (changed && loop.size() > 3)
    {
      changed = false;
      for (const auto i : index_range(loop))
      {
        const std::size_t before = (i + loop.size() - 1) % loop.size();
        if (!edge_side[before] || !edge_side[i])
          continue;

        // A node where the sideset changes is a junction the surgery has to keep, and equal ids
        // are what lets the merged edge inherit them unambiguously
        if (edge_side[before]->ids != edge_side[i]->ids)
          continue;

        const Point a = *loop[before];
        const Point b = *loop[i];
        const Point c = *loop[(i + 1) % loop.size()];

        // Both edges of the node have to be crowded, and the merged edge may not overshoot the
        // target, or the coarsening would trade an over refined boundary for an over coarse one
        const Real target = std::min(edge_side[before]->target, edge_side[i]->target);
        const Real bound = _coarsen_boundary_fraction * target;
        const Real merged_length = (c - a).norm();
        if ((b - a).norm() >= bound || (c - b).norm() >= bound || merged_length > target)
          continue;

        // Dropping the node may not change the geometry of the loop, so it has to sit on the
        // straight line between its neighbors
        if (std::abs(signedArea2(a, b, c)) >
            collinear_deviation_fraction * merged_length * merged_length)
          continue;

        // Both edges at this node are external sides, so every element around it is a cavity
        // element: an element outside the cavity that used it would put more boundary edges
        // through it, which extractBoundaryLoops has already rejected as a pinch. Dropping it
        // from the loop therefore orphans it cleanly.
        ExternalSide merged{edge_side[before]->ids, target};
        loop.erase(loop.begin() + i);
        edge_side.erase(edge_side.begin() + i);
        edge_side[i == 0 ? loop.size() - 1 : i - 1] = std::move(merged);
        changed = true;
        break;
      }
    }

    // A merged edge is not a side of any old element, so triangulateCavity cannot read its
    // sideset off the mesh: it is recorded here for the caller to add to the patch
    for (const auto i : index_range(loop))
    {
      if (!edge_side[i] || edge_side[i]->ids.empty())
        continue;
      const auto node_pair = sortedNodePair(loop[i]->id(), loop[(i + 1) % loop.size()]->id());
      if (!external_sides.count(node_pair))
        merged_side_ids[node_pair] = edge_side[i]->ids;
    }
  }
}

bool
PatchDelaunayRemesher::validateBoundaryLoops(const std::vector<BoundaryLoop> & loops,
                                             Real & mean_edge_length) const
{
  Real total_edge_length = 0;
  Real shortest_edge_length = std::numeric_limits<Real>::max();
  std::size_t n_edges = 0;
  for (const auto & loop : loops)
  {
    if (loop.size() < 3)
      return false;

    for (const auto i : index_range(loop))
    {
      const Real length = (*loop[(i + 1) % loop.size()] - *loop[i]).norm();
      total_edge_length += length;
      shortest_edge_length = std::min(shortest_edge_length, length);
      ++n_edges;
    }
  }

  mean_edge_length = total_edge_length / n_edges;
  if (shortest_edge_length <= collapsed_edge_fraction * mean_edge_length)
    return false;

  const Real length_tol = relative_geometric_tolerance * mean_edge_length;
  const Real area_tol = relative_geometric_tolerance * mean_edge_length * mean_edge_length;

  // The elements of an XY mesh are wound counter-clockwise, so the outer boundary of the cavity
  // comes out counter-clockwise and the boundary of every region it encloses comes out clockwise.
  // Anything else means the deformed cavity has turned itself inside out.
  if (loopSignedArea2(loops[0]) <= area_tol)
    return false;
  for (const auto i : make_range(std::size_t(1), loops.size()))
    if (loopSignedArea2(loops[i]) >= -area_tol)
      return false;

  // Flatten the loops so that every pair of boundary edges can be compared
  std::vector<std::array<Point, 2>> segments;
  std::vector<std::pair<std::size_t, std::size_t>> segment_position;
  for (const auto l : index_range(loops))
    for (const auto i : index_range(loops[l]))
    {
      segments.push_back({Point(*loops[l][i]), Point(*loops[l][(i + 1) % loops[l].size()])});
      segment_position.emplace_back(l, i);
    }

  for (const auto a : index_range(segments))
    for (const auto b : make_range(a + 1, segments.size()))
    {
      const auto & [loop_a, index_a] = segment_position[a];
      const auto & [loop_b, index_b] = segment_position[b];
      if (loop_a == loop_b)
      {
        // Edges that follow each other around a loop share an end by construction
        const std::size_t loop_size = loops[loop_a].size();
        const std::size_t separation = (index_b + loop_size - index_a) % loop_size;
        if (separation == 1 || separation == loop_size - 1)
          continue;
      }

      if (segmentsIntersect(
              segments[a][0], segments[a][1], segments[b][0], segments[b][1], area_tol, length_tol))
        return false;
    }

  return true;
}

bool
PatchDelaunayRemesher::triangulateCavity(const std::vector<dof_id_type> & cavity,
                                         const std::vector<BoundaryLoop> & loops,
                                         const Real mean_edge_length,
                                         CavityPatch & patch) const
{
  auto build_loop_mesh = [this](const BoundaryLoop & loop)
  {
    std::vector<Point> points;
    points.reserve(loop.size());
    for (const Node * node : loop)
      points.push_back(*node);

    std::unique_ptr<MeshBase> loop_mesh = std::make_unique<ReplicatedMesh>(_cavity_comm, 2);
    MooseMeshUtils::buildPolyLineMesh(
        *loop_mesh, points, true, "", "", std::vector<unsigned int>{1});
    return loop_mesh;
  };

  MeshTriangulationUtils::XYDelaunayOptions options;
  options.tri_elem_type = "TRI3";
  // Nothing may subdivide or move the cavity boundary: the new triangles have to meet the
  // surrounding elements at exactly the nodes that are already there
  options.refine_bdy = false;
  options.smooth_tri = false;
  options.refine_holes.assign(loops.size() - 1, false);

  // The triangulator clones the function, but it has to be alive for that, so it outlives the call
  std::unique_ptr<FunctionBase<Real>> sizing_area_function;
  if (_sizing_variable)
    sizing_area_function = buildSizingAreaFunction(cavity);

  // The field replaces the boundary derived target rather than grading it, so use_auto_area_func is
  // left off below and the triangulator is never handed two sizing rules at once. A cavity no
  // element of which carries a target leaves the function null and is sized the way it would be
  // without a sizing variable.
  if (sizing_area_function)
    options.desired_area_function = sizing_area_function.get();
  else if (_desired_area > 0)
    options.desired_area = _desired_area;
  else
  {
    // One target area for the whole cavity only works where the boundary is uniform. The pinned
    // boundary fixes the triangle size along itself, and refine_bdy stops the triangulator from
    // subdividing it, so a target that disagrees with a boundary edge forces slivers against that
    // edge. Grading the target off the length of each boundary edge instead is a fixed point on a
    // uniform boundary and reproduces a graded one.
    options.use_auto_area_func = true;
    options.auto_area_function_num_points = auto_area_num_points;
    options.auto_area_function_power = auto_area_power;
  }

  std::vector<std::unique_ptr<MeshBase>> hole_meshes;
  for (const auto i : make_range(std::size_t(1), loops.size()))
    hole_meshes.push_back(build_loop_mesh(loops[i]));

  std::unique_ptr<MeshBase> triangulation = MeshTriangulationUtils::triangulateWithDelaunay(
      *this, build_loop_mesh(loops[0]), std::move(hole_meshes), options);

  // Every node of the loops is pinned: a triangulated vertex that sits on one of them reuses that
  // very node, which is what carries the sidesets and the subdomain interfaces through untouched
  std::vector<Node *> loop_nodes;
  for (const auto & loop : loops)
    loop_nodes.insert(loop_nodes.end(), loop.begin(), loop.end());
  std::vector<bool> loop_node_pinned(loop_nodes.size(), false);

  const Real match_tol = relative_geometric_tolerance * mean_edge_length;
  const Real area_tol = relative_geometric_tolerance * mean_edge_length * mean_edge_length;

  std::unordered_map<dof_id_type, std::size_t> vertex_of_triangulated_node;
  for (const Elem * elem : triangulation->element_ptr_range())
  {
    if (elem->type() != TRI3 ||
        signedArea2(elem->point(0), elem->point(1), elem->point(2)) <= area_tol)
      return false;

    std::array<std::size_t, 3> triangle;
    for (const auto i : index_range(triangle))
    {
      const Node * triangulated_node = elem->node_ptr(i);
      const auto emplaced =
          vertex_of_triangulated_node.emplace(triangulated_node->id(), patch.vertex_nodes.size());
      if (emplaced.second)
      {
        Node * pinned = nullptr;
        for (const auto n : index_range(loop_nodes))
          if ((*loop_nodes[n] - *triangulated_node).norm() <= match_tol)
          {
            pinned = loop_nodes[n];
            loop_node_pinned[n] = true;
            break;
          }

        patch.vertex_nodes.push_back(pinned);
        patch.vertex_new_point.push_back(pinned ? std::size_t(0) : patch.new_points.size());
        if (!pinned)
          patch.new_points.push_back(*triangulated_node);
      }
      triangle[i] = emplaced.first->second;
    }

    patch.triangles.push_back(triangle);
  }

  // A triangulation that dropped one of the pinned nodes would leave the new triangles hanging off
  // the surrounding mesh, so the cavity is rejected instead
  if (std::find(loop_node_pinned.begin(), loop_node_pinned.end(), false) != loop_node_pinned.end())
    return false;

  patch.old_element_ids = cavity;

  const MeshBase & mesh = _mesh.getMesh();
  const BoundaryInfo & boundary_info = mesh.get_boundary_info();
  std::vector<boundary_id_type> side_ids;
  for (const auto elem_id : cavity)
  {
    const Elem * elem = mesh.elem_ptr(elem_id);
    for (const auto side : elem->side_index_range())
    {
      boundary_info.boundary_ids(elem, side, side_ids);
      if (side_ids.empty())
        continue;

      const auto side_nodes = elem->nodes_on_side(side);
      patch.side_boundary_ids[sortedNodePair(elem->node_id(side_nodes[0]),
                                             elem->node_id(side_nodes[1]))] = side_ids;
    }
  }

  return true;
}

std::vector<Node *>
PatchDelaunayRemesher::collectOrphanedNodes(const std::vector<dof_id_type> & cavity,
                                            const std::vector<BoundaryLoop> & loops) const
{
  MeshBase & mesh = _mesh.getMesh();

  std::set<dof_id_type> boundary_node_ids;
  for (const auto & loop : loops)
    for (const Node * node : loop)
      boundary_node_ids.insert(node->id());

  // A cavity node that no loop uses is used by cavity elements only: an element outside the cavity
  // that shared it would have put it on a loop, because the cavity sides meeting at it would then
  // have been boundary sides
  std::set<dof_id_type> orphaned_ids;
  for (const auto elem_id : cavity)
  {
    const Elem * elem = mesh.elem_ptr(elem_id);
    for (const auto n : elem->node_index_range())
      if (!boundary_node_ids.count(elem->node_id(n)))
        orphaned_ids.insert(elem->node_id(n));
  }

  std::vector<Node *> orphaned;
  orphaned.reserve(orphaned_ids.size());
  for (const auto node_id : orphaned_ids)
    orphaned.push_back(mesh.node_ptr(node_id));

  return orphaned;
}

void
PatchDelaunayRemesher::splicePatch(const CavityPatch & patch,
                                   dof_id_type & next_node_id,
                                   dof_id_type & next_elem_id,
                                   RemeshRecord & record) const
{
  MeshBase & mesh = _mesh.getMesh();

  std::vector<Node *> vertices(patch.vertex_nodes.size(), nullptr);
  for (const auto vertex : index_range(patch.vertex_nodes))
  {
    if (patch.vertex_nodes[vertex])
    {
      vertices[vertex] = patch.vertex_nodes[vertex];
      continue;
    }

    const Point & point = patch.new_points[patch.vertex_new_point[vertex]];
    const RemeshSourcePoint source = locateSourcePoint(patch.old_element_ids, point);
    vertices[vertex] = addNode(point, source, next_node_id, record);
  }

  for (const auto & triangle : patch.triangles)
  {
    const std::array<Node *, 3> nodes{
        vertices[triangle[0]], vertices[triangle[1]], vertices[triangle[2]]};

    Point centroid;
    for (const Node * node : nodes)
      centroid += *node;
    centroid /= 3.0;

    const RemeshSourcePoint source = locateSourcePoint(patch.old_element_ids, centroid);
    addMirroredTriangle(nodes, source, patch.side_boundary_ids, next_elem_id, record);
  }

  // The cavity stays in the mesh. The engine still has to read the old solution through it, erase
  // its stateful material properties and only then free it.
  for (const auto elem_id : patch.old_element_ids)
  {
    Elem * const old_elem = mesh.elem_ptr(elem_id);
    mooseAssert(!_mesh.isDistributedMesh() || old_elem->processor_id() == mesh.processor_id(),
                "Element " << elem_id
                           << " is in a cavity of a rank that does not own it, so this rank is "
                              "about to delete an element another rank owns.");
    record.replaced_elements.push_back(old_elem);
  }

  for (Node * const orphaned_node : patch.orphaned_nodes)
  {
    mooseAssert(!_mesh.isDistributedMesh() || orphaned_node->processor_id() == mesh.processor_id(),
                "Node " << orphaned_node->id()
                        << " lies inside a cavity of a rank that does not own it, so this rank is "
                           "about to delete a node another rank owns.");
    record.replaced_nodes.push_back(orphaned_node);
  }
}
