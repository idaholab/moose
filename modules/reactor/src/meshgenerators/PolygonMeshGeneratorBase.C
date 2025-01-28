//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolygonMeshGeneratorBase.h"
#include "MooseUtils.h"
#include "FormattedTable.h"

#include <cmath>
#include <iomanip>

using namespace libMesh;

InputParameters
PolygonMeshGeneratorBase::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription(
      "A base class that contains common members for Reactor module mesh generators.");

  return params;
}

PolygonMeshGeneratorBase::PolygonMeshGeneratorBase(const InputParameters & parameters)
  : MeshGenerator(parameters)
{
}

std::unique_ptr<MeshBase>
PolygonMeshGeneratorBase::generate()
{
  auto mesh = buildReplicatedMesh(2); // initiate a 2D mesh
  return dynamic_pointer_cast<MeshBase>(mesh);
}

std::unique_ptr<ReplicatedMesh>
PolygonMeshGeneratorBase::buildGeneralSlice(
    std::vector<Real> ring_radii,
    const std::vector<unsigned int> ring_layers,
    const std::vector<Real> ring_radial_biases,
    const multiBdryLayerParams & ring_inner_boundary_layer_params,
    const multiBdryLayerParams & ring_outer_boundary_layer_params,
    std::vector<Real> ducts_center_dist,
    const std::vector<unsigned int> ducts_layers,
    const std::vector<Real> duct_radial_biases,
    const multiBdryLayerParams & duct_inner_boundary_layer_params,
    const multiBdryLayerParams & duct_outer_boundary_layer_params,
    const Real primary_side_length,
    const Real secondary_side_length,
    const unsigned int num_sectors_per_side,
    const unsigned int background_intervals,
    const Real background_radial_bias,
    const singleBdryLayerParams & background_inner_boundary_layer_params,
    const singleBdryLayerParams & background_outer_boundary_layer_params,
    dof_id_type & node_id_background_meta,
    const Real azimuthal_angle,
    const std::vector<Real> azimuthal_tangent,
    const unsigned int side_index,
    const bool quad_center_elements,
    const Real center_quad_factor,
    const Real rotation_angle,
    const bool generate_side_specific_boundaries)
{
  const Real virtual_pitch = 2.0 * primary_side_length * cos(azimuthal_angle / 360.0 * M_PI);
  const Real virtual_side_number = 360.0 / azimuthal_angle;
  const Real pitch_scale_factor = secondary_side_length / primary_side_length;

  auto mesh = buildSlice(ring_radii,
                         ring_layers,
                         ring_radial_biases,
                         ring_inner_boundary_layer_params,
                         ring_outer_boundary_layer_params,
                         ducts_center_dist,
                         ducts_layers,
                         duct_radial_biases,
                         duct_inner_boundary_layer_params,
                         duct_outer_boundary_layer_params,
                         virtual_pitch,
                         num_sectors_per_side,
                         background_intervals,
                         background_radial_bias,
                         background_inner_boundary_layer_params,
                         background_outer_boundary_layer_params,
                         node_id_background_meta,
                         virtual_side_number,
                         side_index,
                         azimuthal_tangent,
                         0,
                         quad_center_elements,
                         center_quad_factor,
                         false,
                         true,
                         0,
                         pitch_scale_factor,
                         generate_side_specific_boundaries);
  MeshTools::Modification::rotate(*mesh, rotation_angle, 0, 0);
  return mesh;
}

std::unique_ptr<ReplicatedMesh>
PolygonMeshGeneratorBase::buildSimpleSlice(
    std::vector<Real> ring_radii,
    const std::vector<unsigned int> ring_layers,
    const std::vector<Real> ring_radial_biases,
    const multiBdryLayerParams & ring_inner_boundary_layer_params,
    const multiBdryLayerParams & ring_outer_boundary_layer_params,
    std::vector<Real> ducts_center_dist,
    const std::vector<unsigned int> ducts_layers,
    const std::vector<Real> duct_radial_biases,
    const multiBdryLayerParams & duct_inner_boundary_layer_params,
    const multiBdryLayerParams & duct_outer_boundary_layer_params,
    const Real pitch,
    const unsigned int num_sectors_per_side,
    const unsigned int background_intervals,
    const Real background_radial_bias,
    const singleBdryLayerParams & background_inner_boundary_layer_params,
    const singleBdryLayerParams & background_outer_boundary_layer_params,
    dof_id_type & node_id_background_meta,
    const unsigned int side_number,
    const unsigned int side_index,
    const std::vector<Real> azimuthal_tangent,
    const subdomain_id_type block_id_shift,
    const bool quad_center_elements,
    const Real center_quad_factor,
    const bool create_inward_interface_boundaries,
    const bool create_outward_interface_boundaries,
    const boundary_id_type boundary_id_shift,
    const bool generate_side_specific_boundaries,
    const TRI_ELEM_TYPE tri_elem_type,
    const QUAD_ELEM_TYPE quad_elem_type)
{
  return buildSlice(ring_radii,
                    ring_layers,
                    ring_radial_biases,
                    ring_inner_boundary_layer_params,
                    ring_outer_boundary_layer_params,
                    ducts_center_dist,
                    ducts_layers,
                    duct_radial_biases,
                    duct_inner_boundary_layer_params,
                    duct_outer_boundary_layer_params,
                    pitch,
                    num_sectors_per_side,
                    background_intervals,
                    background_radial_bias,
                    background_inner_boundary_layer_params,
                    background_outer_boundary_layer_params,
                    node_id_background_meta,
                    side_number,
                    side_index,
                    azimuthal_tangent,
                    block_id_shift,
                    quad_center_elements,
                    center_quad_factor,
                    create_inward_interface_boundaries,
                    create_outward_interface_boundaries,
                    boundary_id_shift,
                    1.0,
                    generate_side_specific_boundaries,
                    tri_elem_type,
                    quad_elem_type);
}

std::unique_ptr<ReplicatedMesh>
PolygonMeshGeneratorBase::buildSlice(
    std::vector<Real> ring_radii,
    const std::vector<unsigned int> ring_layers,
    const std::vector<Real> ring_radial_biases,
    const multiBdryLayerParams & ring_inner_boundary_layer_params,
    const multiBdryLayerParams & ring_outer_boundary_layer_params,
    std::vector<Real> ducts_center_dist,
    const std::vector<unsigned int> ducts_layers,
    const std::vector<Real> duct_radial_biases,
    const multiBdryLayerParams & duct_inner_boundary_layer_params,
    const multiBdryLayerParams & duct_outer_boundary_layer_params,
    const Real pitch,
    const unsigned int num_sectors_per_side,
    const unsigned int background_intervals,
    const Real background_radial_bias,
    const singleBdryLayerParams & background_inner_boundary_layer_params,
    const singleBdryLayerParams & background_outer_boundary_layer_params,
    dof_id_type & node_id_background_meta,
    const Real virtual_side_number,
    const unsigned int side_index,
    const std::vector<Real> azimuthal_tangent,
    const subdomain_id_type block_id_shift,
    const bool quad_center_elements,
    const Real center_quad_factor,
    const bool create_inward_interface_boundaries,
    const bool create_outward_interface_boundaries,
    const boundary_id_type boundary_id_shift,
    const Real pitch_scale_factor,
    const bool generate_side_specific_boundaries,
    const TRI_ELEM_TYPE tri_elem_type,
    const QUAD_ELEM_TYPE quad_elem_type)
{
  const unsigned short order = quad_elem_type == QUAD_ELEM_TYPE::QUAD4 ? 1 : 2;
  if (order != (tri_elem_type == TRI_ELEM_TYPE::TRI3 ? 1 : 2))
    mooseError("In mesh generator ",
               this->name(),
               ", an incompatible elements type combination is used when calling "
               "PolygonMeshGeneratorBase::buildSlice().");
  // In order to create quadratic elements (i.e., order = 2), we creates nodes with double mesh
  // density. Thus, the related parameters need to be modified accordingly. A prefix "mod_" is used
  // to indicate the modified parameters.

  // For ring_layers, modification is to double the number of layers for order = 2
  std::vector<unsigned int> mod_ring_layers(ring_layers);
  std::for_each(
      mod_ring_layers.begin(), mod_ring_layers.end(), [&order](unsigned int & n) { n *= order; });
  // For ring_radial_biases, modification is to take the square root of the original biases for
  // order = 2
  std::vector<Real> mod_ring_radial_biases(ring_radial_biases);
  std::for_each(mod_ring_radial_biases.begin(),
                mod_ring_radial_biases.end(),
                [&order](Real & n) { n = std::pow(n, 1.0 / order); });
  // ducts_layers is similar to ring_layers
  std::vector<unsigned int> mod_ducts_layers(ducts_layers);
  std::for_each(
      mod_ducts_layers.begin(), mod_ducts_layers.end(), [&order](unsigned int & n) { n *= order; });
  // duct_radial_biases is similar to ring_radial_biases
  std::vector<Real> mod_duct_radial_biases(duct_radial_biases);
  std::for_each(mod_duct_radial_biases.begin(),
                mod_duct_radial_biases.end(),
                [&order](Real & n) { n = std::pow(n, 1.0 / order); });
  // Azimuthal mesh density is also doubled for order = 2
  const unsigned int mod_num_sectors_per_side = num_sectors_per_side * order;
  const unsigned int mod_background_intervals = background_intervals * order;
  // background_radial_bias is similar to ring_radial_biases
  const Real mod_background_radial_bias = std::pow(background_radial_bias, 1.0 / order);
  // Perform similar modifications for boundary layer parameters
  const auto mod_ring_inner_boundary_layer_params =
      modifiedMultiBdryLayerParamsCreator(ring_inner_boundary_layer_params, order);
  const auto mod_ring_outer_boundary_layer_params =
      modifiedMultiBdryLayerParamsCreator(ring_outer_boundary_layer_params, order);
  const auto mod_duct_inner_boundary_layer_params =
      modifiedMultiBdryLayerParamsCreator(duct_inner_boundary_layer_params, order);
  const auto mod_duct_outer_boundary_layer_params =
      modifiedMultiBdryLayerParamsCreator(duct_outer_boundary_layer_params, order);

  const auto mod_background_inner_boundary_layer_params =
      modifiedSingleBdryLayerParamsCreator(background_inner_boundary_layer_params, order);
  const auto mod_background_outer_boundary_layer_params =
      modifiedSingleBdryLayerParamsCreator(background_outer_boundary_layer_params, order);

  // The distance parameters of the rings and duct need to be modified too as they may be involved
  // in the boundary layer cases.
  std::vector<Real> mod_ducts_center_dist(ducts_center_dist);
  std::vector<Real> mod_ring_radii(ring_radii);
  bool has_rings(ring_radii.size());
  bool has_ducts(ducts_center_dist.size());
  bool has_background(background_intervals);
  auto mesh = buildReplicatedMesh(2);

  // Calculate biasing terms
  // background region needs to be split into three parts
  const auto main_background_bias_terms =
      biasTermsCalculator(background_radial_bias, background_intervals);
  const auto inner_background_bias_terms =
      biasTermsCalculator(background_inner_boundary_layer_params.bias,
                          background_inner_boundary_layer_params.intervals);
  const auto outer_background_bias_terms =
      biasTermsCalculator(background_outer_boundary_layer_params.bias,
                          background_outer_boundary_layer_params.intervals);
  auto rings_bias_terms = biasTermsCalculator(ring_radial_biases,
                                              ring_layers,
                                              ring_inner_boundary_layer_params,
                                              ring_outer_boundary_layer_params);
  auto duct_bias_terms = biasTermsCalculator(duct_radial_biases,
                                             ducts_layers,
                                             duct_inner_boundary_layer_params,
                                             duct_outer_boundary_layer_params);
  // Equivalent "mod_" parts
  const auto mod_main_background_bias_terms =
      biasTermsCalculator(mod_background_radial_bias, mod_background_intervals);
  const auto mod_inner_background_bias_terms =
      biasTermsCalculator(mod_background_inner_boundary_layer_params.bias,
                          mod_background_inner_boundary_layer_params.intervals);
  const auto mod_outer_background_bias_terms =
      biasTermsCalculator(mod_background_outer_boundary_layer_params.bias,
                          mod_background_outer_boundary_layer_params.intervals);
  auto mod_rings_bias_terms = biasTermsCalculator(mod_ring_radial_biases,
                                                  mod_ring_layers,
                                                  mod_ring_inner_boundary_layer_params,
                                                  mod_ring_outer_boundary_layer_params);
  auto mod_duct_bias_terms = biasTermsCalculator(mod_duct_radial_biases,
                                                 mod_ducts_layers,
                                                 mod_duct_inner_boundary_layer_params,
                                                 mod_duct_outer_boundary_layer_params);

  std::vector<unsigned int> total_ring_layers;
  for (unsigned int i = 0; i < ring_layers.size(); i++)
    total_ring_layers.push_back(ring_layers[i] + ring_inner_boundary_layer_params.intervals[i] +
                                ring_outer_boundary_layer_params.intervals[i]);

  if (background_inner_boundary_layer_params.intervals)
  {
    total_ring_layers.push_back(background_inner_boundary_layer_params.intervals);
    rings_bias_terms.push_back(inner_background_bias_terms);
    ring_radii.push_back((ring_radii.empty() ? 0.0 : ring_radii.back()) +
                         background_inner_boundary_layer_params.width);
    has_rings = true;
  }
  std::vector<unsigned int> mod_total_ring_layers;
  for (unsigned int i = 0; i < mod_ring_layers.size(); i++)
    mod_total_ring_layers.push_back(mod_ring_layers[i] +
                                    mod_ring_inner_boundary_layer_params.intervals[i] +
                                    mod_ring_outer_boundary_layer_params.intervals[i]);

  if (mod_background_inner_boundary_layer_params.intervals)
  {
    mod_total_ring_layers.push_back(mod_background_inner_boundary_layer_params.intervals);
    mod_rings_bias_terms.push_back(mod_inner_background_bias_terms);
    mod_ring_radii.push_back((mod_ring_radii.empty() ? 0.0 : mod_ring_radii.back()) +
                             mod_background_inner_boundary_layer_params.width);
    // has_rings should be modified before in the none "mod_" part
  }

  std::vector<unsigned int> total_ducts_layers;
  if (background_outer_boundary_layer_params.intervals)
  {
    total_ducts_layers.push_back(background_outer_boundary_layer_params.intervals);
    duct_bias_terms.insert(duct_bias_terms.begin(), outer_background_bias_terms);
    ducts_center_dist.insert(ducts_center_dist.begin(),
                             (ducts_center_dist.empty()
                                  ? pitch / 2.0 / std::cos(M_PI / virtual_side_number)
                                  : ducts_center_dist.front()) -
                                 background_outer_boundary_layer_params.width);
    has_ducts = true;
  }
  for (unsigned int i = 0; i < ducts_layers.size(); i++)
    total_ducts_layers.push_back(ducts_layers[i] + duct_inner_boundary_layer_params.intervals[i] +
                                 duct_outer_boundary_layer_params.intervals[i]);

  std::vector<unsigned int> mod_total_ducts_layers;
  if (mod_background_outer_boundary_layer_params.intervals)
  {
    mod_total_ducts_layers.push_back(mod_background_outer_boundary_layer_params.intervals);
    mod_duct_bias_terms.insert(mod_duct_bias_terms.begin(), mod_outer_background_bias_terms);
    mod_ducts_center_dist.insert(mod_ducts_center_dist.begin(),
                                 (mod_ducts_center_dist.empty()
                                      ? pitch / 2.0 / std::cos(M_PI / virtual_side_number)
                                      : mod_ducts_center_dist.front()) -
                                     mod_background_outer_boundary_layer_params.width);
    // has_ducts should be modified before in the none "mod_" part
  }
  for (unsigned int i = 0; i < mod_ducts_layers.size(); i++)
    mod_total_ducts_layers.push_back(mod_ducts_layers[i] +
                                     mod_duct_inner_boundary_layer_params.intervals[i] +
                                     mod_duct_outer_boundary_layer_params.intervals[i]);

  unsigned int angle_number = azimuthal_tangent.size() == 0
                                  ? num_sectors_per_side
                                  : ((azimuthal_tangent.size() - 1) / order);
  unsigned int mod_angle_number =
      azimuthal_tangent.size() == 0 ? mod_num_sectors_per_side : (azimuthal_tangent.size() - 1);

  // Geometries
  const Real corner_to_corner =
      pitch / std::cos(M_PI / virtual_side_number); // distance of bin center to cell corner
  const Real corner_p[2][2] = {
      {0.0, 0.5 * corner_to_corner},
      {0.5 * corner_to_corner * pitch_scale_factor * std::sin(2.0 * M_PI / virtual_side_number),
       0.5 * corner_to_corner * pitch_scale_factor * std::cos(2.0 * M_PI / virtual_side_number)}};
  const unsigned int div_num = angle_number / 2 + 1;
  const unsigned int mod_div_num = mod_angle_number / 2 + 1;

  // From now on, we work on the nodes, which need the "mod_" parameters
  std::vector<std::vector<Node *>> nodes(mod_div_num, std::vector<Node *>(mod_div_num));
  if (quad_center_elements)
  {
    Real ring_radii_0;

    if (has_rings)
      ring_radii_0 = ring_radii.front() * mod_rings_bias_terms.front()[order - 1];
    else if (has_ducts)
      ring_radii_0 = mod_ducts_center_dist.front() * std::cos(M_PI / virtual_side_number) *
                     mod_main_background_bias_terms[order - 1];
    else
      ring_radii_0 = pitch / 2.0 * mod_main_background_bias_terms[order - 1];
    // If center_quad_factor is zero, default value (div_num - 1)/div_num  is used.
    // We use div_num instead of mod_div_num because we are dealing wth elements here
    // This approach ensures that the order = 2 mesh elements are consistent with the order = 1
    ring_radii_0 *=
        center_quad_factor == 0.0 ? (((Real)div_num - 1.0) / (Real)div_num) : center_quad_factor;

    centerNodes(*mesh, virtual_side_number, mod_div_num, ring_radii_0, nodes);
  }
  else // pin-cell center
    mesh->add_point(Point(0.0, 0.0, 0.0));

  // create nodes for the ring regions
  if (has_rings)
    ringNodes(*mesh,
              ring_radii,
              mod_total_ring_layers,
              mod_rings_bias_terms,
              mod_num_sectors_per_side,
              corner_p,
              corner_to_corner,
              azimuthal_tangent);

  if (has_background)
  {
    // add nodes in background region; the background region is defined as the area between the
    // outermost pin (if there is a pin; if no pin, the center) and the innermost hex/duct; if
    // _has_ducts is false, the background region is the area between the pin and enclosing hexagon
    Real background_corner_radial_interval_length;
    Real background_corner_distance;
    Real background_in;
    Real background_out; // background outer frontier
    if (has_rings)
      background_in = ring_radii.back();
    else
      background_in = 0;

    if (has_ducts)
    {
      background_out = mod_ducts_center_dist.front();
      background_corner_distance =
          mod_ducts_center_dist
              .front(); // it is the center to duct (innermost duct) corner distance
    }
    else
    {
      background_out = 0.5 * corner_to_corner;
      background_corner_distance =
          0.5 * corner_to_corner; // it is the center to hex corner distance
    }

    background_corner_radial_interval_length =
        (background_out - background_in) / mod_background_intervals;

    node_id_background_meta = mesh->n_nodes();

    // create nodes for background region
    backgroundNodes(*mesh,
                    mod_num_sectors_per_side,
                    mod_background_intervals,
                    mod_main_background_bias_terms,
                    background_corner_distance,
                    background_corner_radial_interval_length,
                    corner_p,
                    corner_to_corner,
                    background_in,
                    azimuthal_tangent);
  }

  // create nodes for duct regions
  if (has_ducts)
    ductNodes(*mesh,
              &mod_ducts_center_dist,
              mod_total_ducts_layers,
              mod_duct_bias_terms,
              mod_num_sectors_per_side,
              corner_p,
              corner_to_corner,
              azimuthal_tangent);

  // See if the central region is the only part of the innermost part
  // The central region of the slice is special.
  // Unlike the outer regions, which are layered quad elements,
  // the central region is either a layer of tri elements or a specially-patterned quad elements.
  // If there is at least one `ring` defined in the slice,
  // the central region must belong to the innermost (first) ring.
  // Otherwise the central region belongs to the `background`
  // In either case, if the innermost ring or background has only one radial interval,
  // the central region is an independent ring or background
  // Otherwise, the central region and one or several quad element layers together form the
  // innermost ring or background
  bool is_central_region_independent;
  if (ring_layers.empty())
    is_central_region_independent = mod_background_inner_boundary_layer_params.intervals +
                                        mod_background_intervals +
                                        mod_background_outer_boundary_layer_params.intervals ==
                                    1;
  else
    is_central_region_independent = mod_ring_layers[0] +
                                        mod_ring_inner_boundary_layer_params.intervals[0] +
                                        mod_ring_outer_boundary_layer_params.intervals[0] ==
                                    1;

  // From now on, we work on the elements, which need the none "mod_" parameters
  // Assign elements, boundaries, and subdomains;
  // Add Tri3/Tri6/Tri7 or Quad4/Quad8/Quad9 mesh into innermost (central) region
  if (quad_center_elements)
    cenQuadElemDef(*mesh,
                   div_num,
                   block_id_shift,
                   create_outward_interface_boundaries && is_central_region_independent,
                   boundary_id_shift,
                   nodes,
                   (!has_rings) && (!has_ducts) && (background_intervals == 1),
                   // Note here, has_ring means either there are ring regions or background inner
                   // boundary layer; has_ducts means either there are duct regions or background
                   // outer boundary layer. Same in cenTriElemDef()
                   side_index,
                   generate_side_specific_boundaries,
                   quad_elem_type);
  else
    cenTriElemDef(
        *mesh,
        num_sectors_per_side,
        azimuthal_tangent,
        block_id_shift,
        create_outward_interface_boundaries && is_central_region_independent,
        boundary_id_shift,
        ((!has_rings) && (!has_ducts) && (background_intervals == 1)) ||
            ((!has_background) &&
             (std::accumulate(total_ring_layers.begin(), total_ring_layers.end(), 0) == 1)),
        // Only for ACCG, it is possible that the entire mesh is a single-layer ring.
        // cenQuadElemDef() does not need this as it does not work for ACCG.
        side_index,
        generate_side_specific_boundaries,
        tri_elem_type);

  // Add Quad4 mesh into outer circle
  // total number of mesh should be all the rings for pin regions + background regions;
  // total number of quad mesh should be total number of mesh -1 (-1 is because the inner circle for
  // tri/quad mesh has been added above)

  std::vector<unsigned int> subdomain_rings;
  if (has_rings) //  define the rings in each subdomain
  {
    subdomain_rings = total_ring_layers;
    subdomain_rings.front() -= 1; // remove the inner TRI mesh subdomain
    if (background_inner_boundary_layer_params.intervals)
    {
      subdomain_rings.back() =
          background_inner_boundary_layer_params.intervals + background_intervals +
          background_outer_boundary_layer_params.intervals; // add the background region
      if (ring_radii.size() == 1)
        subdomain_rings.back() -= 1; // remove the inner TRI mesh subdomain
    }
    else if (has_background)
      subdomain_rings.push_back(background_inner_boundary_layer_params.intervals +
                                background_intervals +
                                background_outer_boundary_layer_params.intervals);
  }
  else
  {
    subdomain_rings.push_back(
        background_inner_boundary_layer_params.intervals + background_intervals +
        background_outer_boundary_layer_params.intervals); // add the background region
    subdomain_rings[0] -= 1;                               // remove the inner TRI mesh subdomain
  }

  if (has_ducts)
    for (unsigned int i = (background_outer_boundary_layer_params.intervals > 0);
         i < total_ducts_layers.size();
         i++)
      subdomain_rings.push_back(total_ducts_layers[i]);

  quadElemDef(*mesh,
              num_sectors_per_side,
              subdomain_rings,
              side_index,
              azimuthal_tangent,
              block_id_shift,
              quad_center_elements ? (mod_div_num * mod_div_num - 1) : 0,
              create_inward_interface_boundaries,
              create_outward_interface_boundaries,
              boundary_id_shift,
              generate_side_specific_boundaries,
              quad_elem_type);
  if (tri_elem_type == TRI_ELEM_TYPE::TRI6 || quad_elem_type == QUAD_ELEM_TYPE::QUAD8)
    mesh->remove_orphaned_nodes();
  return mesh;
}

void
PolygonMeshGeneratorBase::centerNodes(ReplicatedMesh & mesh,
                                      const Real virtual_side_number,
                                      const unsigned int div_num,
                                      const Real ring_radii_0,
                                      std::vector<std::vector<Node *>> & nodes) const
{
  const std::pair<Real, Real> p_origin = std::make_pair(0.0, 0.0);
  const std::pair<Real, Real> p_bottom =
      std::make_pair(0.0, ring_radii_0 * std::cos(M_PI / virtual_side_number));
  const std::pair<Real, Real> p_top =
      std::make_pair(p_bottom.second * std::sin(2.0 * M_PI / virtual_side_number),
                     p_bottom.second * std::cos(2.0 * M_PI / virtual_side_number));
  const std::pair<Real, Real> p_diag =
      std::make_pair(ring_radii_0 * std::sin(M_PI / virtual_side_number),
                     ring_radii_0 * std::cos(M_PI / virtual_side_number));

  // The four vertices of the central quad region are defined above.
  // The following loops transverse all the nodes within this central quad region by moving p1 thru
  // p4 and calculate the four-point intercept (pc).
  //  p_top------o-------p4--------o-----p_diag
  //    |        |        |        |        |
  //    |        |        |        |        |
  //    o--------o--------o--------o--------o
  //    |        |        |        |        |
  //    |        |        |        |        |
  //   p1--------o-------pc--------o-------p2
  //    |        |        |        |        |
  //    |        |        |        |        |
  //    o--------o--------o--------o--------o
  //    |        |        |        |        |
  //    |        |        |        |        |
  // p_origin-----o-------p3--------o----p_bottom
  //
  // The loops are designed to transverse the nodes as shown below to facilitate elements
  // and sides creation.
  //
  //   25-------24-------23-------22-------21
  //    |        |        |        |        |
  //    |        |        |        |        |
  //   16-------15-------14-------13-------20
  //    |        |        |        |        |
  //    |        |        |        |        |
  //    9--------8------- 7-------12-------19
  //    |        |        |        |        |
  //    |        |        |        |        |
  //    4--------3--------6-------11-------18
  //    |        |        |        |        |
  //    |        |        |        |        |
  //    1--------2--------5-------10-------17

  for (unsigned int i = 0; i < div_num; i++)
  {
    unsigned int id_x = 0;
    unsigned int id_y = i;
    for (unsigned int j = 0; j < 2 * i + 1; j++)
    {
      std::pair<Real, Real> p1 = std::make_pair(
          (p_origin.first * (div_num - 1 - id_x) + p_top.first * id_x) / (div_num - 1),
          (p_origin.second * (div_num - 1 - id_x) + p_top.second * id_x) / (div_num - 1));
      std::pair<Real, Real> p2 = std::make_pair(
          (p_bottom.first * (div_num - 1 - id_x) + p_diag.first * id_x) / (div_num - 1),
          (p_bottom.second * (div_num - 1 - id_x) + p_diag.second * id_x) / (div_num - 1));
      std::pair<Real, Real> p3 = std::make_pair(
          (p_origin.first * (div_num - 1 - id_y) + p_bottom.first * id_y) / (div_num - 1),
          (p_origin.second * (div_num - 1 - id_y) + p_bottom.second * id_y) / (div_num - 1));
      std::pair<Real, Real> p4 = std::make_pair(
          (p_top.first * (div_num - 1 - id_y) + p_diag.first * id_y) / (div_num - 1),
          (p_top.second * (div_num - 1 - id_y) + p_diag.second * id_y) / (div_num - 1));
      std::pair<Real, Real> pc = fourPointIntercept(p1, p2, p3, p4);
      nodes[id_x][id_y] = mesh.add_point(Point(pc.first, pc.second, 0.0));
      if (j < i)
        id_x++;
      if (j >= i)
        id_y--;
    }
  }
}

void
PolygonMeshGeneratorBase::ringNodes(ReplicatedMesh & mesh,
                                    const std::vector<Real> ring_radii,
                                    const std::vector<unsigned int> ring_layers,
                                    const std::vector<std::vector<Real>> biased_terms,
                                    const unsigned int num_sectors_per_side,
                                    const Real corner_p[2][2],
                                    const Real corner_to_corner,
                                    const std::vector<Real> azimuthal_tangent) const
{
  const unsigned int angle_number =
      azimuthal_tangent.size() == 0 ? num_sectors_per_side : (azimuthal_tangent.size() - 1);

  // Add nodes in pins regions
  for (unsigned int l = 0; l < ring_layers.size(); l++)
  {
    // the pin radius interval for each ring_radii/subdomain
    const Real pin_radius_interval_length =
        l == 0 ? ring_radii[l] / ring_layers[l]
               : (ring_radii[l] - ring_radii[l - 1]) / ring_layers[l];

    // add rings in each pin subdomain
    for (unsigned int k = 0; k < ring_layers[l]; k++)
    {
      const Real bin_radial_distance =
          l == 0 ? (biased_terms[l][k] * ring_layers[l] *
                    pin_radius_interval_length) // this is from the cell/pin center to
                                                // the first circle
                 : (ring_radii[l - 1] +
                    biased_terms[l][k] * ring_layers[l] * pin_radius_interval_length);
      const Real pin_corner_p_x = corner_p[0][0] * bin_radial_distance / (0.5 * corner_to_corner);
      const Real pin_corner_p_y = corner_p[0][1] * bin_radial_distance / (0.5 * corner_to_corner);

      // pin_corner_p(s) are the points in the pin region, on the bins towards the six corners,
      // at different intervals
      mesh.add_point(Point(pin_corner_p_x, pin_corner_p_y, 0.0));

      for (unsigned int j = 1; j <= angle_number; j++)
      {
        const Real cell_boundary_p_x =
            corner_p[0][0] + (corner_p[1][0] - corner_p[0][0]) *
                                 (azimuthal_tangent.size() == 0 ? ((Real)j / (Real)angle_number)
                                                                : (azimuthal_tangent[j] / 2.0));
        const Real cell_boundary_p_y =
            corner_p[0][1] + (corner_p[1][1] - corner_p[0][1]) *
                                 (azimuthal_tangent.size() == 0 ? ((Real)j / (Real)angle_number)
                                                                : (azimuthal_tangent[j] / 2.0));
        // cell_boundary_p(s) are the points on the cell's six boundaries (flat sides) at
        // different azimuthal angles
        const Real pin_azimuthal_p_x =
            cell_boundary_p_x * bin_radial_distance /
            std::sqrt(Utility::pow<2>(cell_boundary_p_x) + Utility::pow<2>(cell_boundary_p_y));
        const Real pin_azimuthal_p_y =
            cell_boundary_p_y * bin_radial_distance /
            std::sqrt(Utility::pow<2>(cell_boundary_p_x) + Utility::pow<2>(cell_boundary_p_y));

        // pin_azimuthal_p are the points on the bins towards different azimuthal angles, at
        // different intervals; excluding the ones produced by pin_corner_p
        mesh.add_point(Point(pin_azimuthal_p_x, pin_azimuthal_p_y, 0.0));
      }
    }
  }
}

void
PolygonMeshGeneratorBase::backgroundNodes(ReplicatedMesh & mesh,
                                          const unsigned int num_sectors_per_side,
                                          const unsigned int background_intervals,
                                          const std::vector<Real> biased_terms,
                                          const Real background_corner_distance,
                                          const Real background_corner_radial_interval_length,
                                          const Real corner_p[2][2],
                                          const Real corner_to_corner,
                                          const Real background_in,
                                          const std::vector<Real> azimuthal_tangent) const
{
  unsigned int angle_number =
      azimuthal_tangent.size() == 0 ? num_sectors_per_side : (azimuthal_tangent.size() - 1);
  for (unsigned int k = 0; k < (background_intervals); k++)
  {
    const Real background_corner_p_x =
        background_corner_distance / (0.5 * corner_to_corner) * corner_p[0][0] *
        (background_in +
         biased_terms[k] * background_intervals * background_corner_radial_interval_length) /
        background_corner_distance;
    const Real background_corner_p_y =
        background_corner_distance / (0.5 * corner_to_corner) * corner_p[0][1] *
        (background_in +
         biased_terms[k] * background_intervals * background_corner_radial_interval_length) /
        background_corner_distance;

    // background_corner_p(s) are the points in the background region, on the bins towards the six
    // corners, at different intervals
    mesh.add_point(Point(background_corner_p_x, background_corner_p_y, 0.0));

    for (unsigned int j = 1; j <= angle_number; j++)
    {
      const Real cell_boundary_p_x =
          background_corner_distance / (0.5 * corner_to_corner) *
          (corner_p[0][0] + (corner_p[1][0] - corner_p[0][0]) *
                                (azimuthal_tangent.size() == 0 ? ((Real)j / (Real)angle_number)
                                                               : (azimuthal_tangent[j] / 2.0)));
      const Real cell_boundary_p_y =
          background_corner_distance / (0.5 * corner_to_corner) *
          (corner_p[0][1] + (corner_p[1][1] - corner_p[0][1]) *
                                (azimuthal_tangent.size() == 0 ? ((Real)j / (Real)angle_number)
                                                               : (azimuthal_tangent[j] / 2.0)));
      // cell_boundary_p(s) are the points on the cell's six boundaries (flat sides) at different
      // azimuthal angles
      const Real pin_boundary_p_x =
          cell_boundary_p_x * background_in /
          std::sqrt(Utility::pow<2>(cell_boundary_p_x) + Utility::pow<2>(cell_boundary_p_y));
      const Real pin_boundary_p_y =
          cell_boundary_p_y * background_in /
          std::sqrt(Utility::pow<2>(cell_boundary_p_x) + Utility::pow<2>(cell_boundary_p_y));
      // pin_boundary_p(s) are the points on pin boundary (outside ring) at different azimuthal
      // angles
      const Real background_radial_interval =
          std::sqrt(Utility::pow<2>(cell_boundary_p_x - pin_boundary_p_x) +
                    Utility::pow<2>(cell_boundary_p_y - pin_boundary_p_y)) /
          background_intervals;
      const Real background_azimuthal_p_x =
          cell_boundary_p_x *
          (background_in + biased_terms[k] * background_intervals * background_radial_interval) /
          std::sqrt(Utility::pow<2>(cell_boundary_p_x) + Utility::pow<2>(cell_boundary_p_y));
      const Real background_azimuthal_p_y =
          cell_boundary_p_y *
          (background_in + biased_terms[k] * background_intervals * background_radial_interval) /
          std::sqrt(Utility::pow<2>(cell_boundary_p_x) + Utility::pow<2>(cell_boundary_p_y));
      // background_azimuthal_p are the points on the bins towards different azimuthal angles, at
      // different intervals; excluding the ones produced by background_corner_p
      mesh.add_point(Point(background_azimuthal_p_x, background_azimuthal_p_y, 0.0));
    }
  }
}

void
PolygonMeshGeneratorBase::ductNodes(ReplicatedMesh & mesh,
                                    std::vector<Real> * const ducts_center_dist,
                                    const std::vector<unsigned int> ducts_layers,
                                    const std::vector<std::vector<Real>> biased_terms,
                                    const unsigned int num_sectors_per_side,
                                    const Real corner_p[2][2],
                                    const Real corner_to_corner,
                                    const std::vector<Real> azimuthal_tangent) const
{
  unsigned int angle_number =
      azimuthal_tangent.size() == 0 ? num_sectors_per_side : (azimuthal_tangent.size() - 1);
  // Add nodes in ducts regions
  (*ducts_center_dist)
      .push_back(0.5 * corner_to_corner); // add hex boundary as the last element in this vector
  std::vector<Real> duct_radius_interval_length(ducts_layers.size());

  Real bin_radial_distance;
  for (unsigned int l = 0; l < ducts_layers.size(); l++)
  {
    duct_radius_interval_length[l] =
        ((*ducts_center_dist)[l + 1] - (*ducts_center_dist)[l]) /
        ducts_layers[l]; // the pin radius interval for each ring_radii/subdomain

    // add rings in each pin subdomain
    for (unsigned int k = 0; k < ducts_layers[l]; k++)
    {
      bin_radial_distance = ((*ducts_center_dist)[l] +
                             biased_terms[l][k] * ducts_layers[l] * duct_radius_interval_length[l]);
      const Real pin_corner_p_x = corner_p[0][0] * bin_radial_distance / (0.5 * corner_to_corner);
      const Real pin_corner_p_y = corner_p[0][1] * bin_radial_distance / (0.5 * corner_to_corner);

      // pin_corner_p(s) are the points in the pin region, on the bins towards the six corners,
      // at different intervals
      mesh.add_point(Point(pin_corner_p_x, pin_corner_p_y, 0.0));

      for (unsigned int j = 1; j <= angle_number; j++)
      {
        const Real cell_boundary_p_x =
            corner_p[0][0] + (corner_p[1][0] - corner_p[0][0]) *
                                 (azimuthal_tangent.size() == 0 ? ((Real)j / (Real)angle_number)
                                                                : (azimuthal_tangent[j] / 2.0));
        const Real cell_boundary_p_y =
            corner_p[0][1] + (corner_p[1][1] - corner_p[0][1]) *
                                 (azimuthal_tangent.size() == 0 ? ((Real)j / (Real)angle_number)
                                                                : (azimuthal_tangent[j] / 2.0));
        // cell_boundary_p(s) are the points on the cell's six boundaries (flat sides) at
        // different azimuthal angles
        const Real pin_azimuthal_p_x =
            cell_boundary_p_x * bin_radial_distance / (0.5 * corner_to_corner);
        const Real pin_azimuthal_p_y =
            cell_boundary_p_y * bin_radial_distance / (0.5 * corner_to_corner);

        // pin_azimuthal_p are the points on the bins towards different azimuthal angles, at
        // different intervals; excluding the ones produced by pin_corner_p
        mesh.add_point(Point(pin_azimuthal_p_x, pin_azimuthal_p_y, 0.0));
      }
    }
  }
}

void
PolygonMeshGeneratorBase::cenQuadElemDef(ReplicatedMesh & mesh,
                                         const unsigned int div_num,
                                         const subdomain_id_type block_id_shift,
                                         const bool create_outward_interface_boundaries,
                                         const boundary_id_type boundary_id_shift,
                                         std::vector<std::vector<Node *>> & nodes,
                                         const bool assign_external_boundary,
                                         const unsigned int side_index,
                                         const bool generate_side_specific_boundaries,
                                         const QUAD_ELEM_TYPE quad_elem_type) const
{

  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  // This loop defines quad elements for the central regions except for the outermost layer
  for (unsigned int i = 0; i < div_num - 1; i++)
  {
    unsigned int id_x = 0;
    unsigned int id_y = i;
    for (unsigned int j = 0; j < 2 * i + 1; j++)
    {
      std::unique_ptr<Elem> new_elem;
      if (quad_elem_type == QUAD_ELEM_TYPE::QUAD4)
      {
        new_elem = std::make_unique<Quad4>();
        new_elem->set_node(0) = nodes[id_x][id_y];
        new_elem->set_node(3) = nodes[id_x][id_y + 1];
        new_elem->set_node(2) = nodes[id_x + 1][id_y + 1];
        new_elem->set_node(1) = nodes[id_x + 1][id_y];
        new_elem->subdomain_id() = 1 + block_id_shift;
      }
      else // QUAD8/QUAD9
      {
        new_elem = std::make_unique<Quad8>();
        if (quad_elem_type == QUAD_ELEM_TYPE::QUAD9)
        {
          new_elem = std::make_unique<Quad9>();
          new_elem->set_node(8) = nodes[id_x * 2 + 1][id_y * 2 + 1];
        }
        new_elem->set_node(0) = nodes[id_x * 2][id_y * 2];
        new_elem->set_node(3) = nodes[id_x * 2][id_y * 2 + 2];
        new_elem->set_node(2) = nodes[id_x * 2 + 2][id_y * 2 + 2];
        new_elem->set_node(1) = nodes[id_x * 2 + 2][id_y * 2];
        new_elem->set_node(4) = nodes[id_x * 2 + 1][id_y * 2];
        new_elem->set_node(5) = nodes[id_x * 2 + 2][id_y * 2 + 1];
        new_elem->set_node(6) = nodes[id_x * 2 + 1][id_y * 2 + 2];
        new_elem->set_node(7) = nodes[id_x * 2][id_y * 2 + 1];
        new_elem->subdomain_id() = 1 + block_id_shift;
      }
      Elem * elem_Quad = mesh.add_elem(std::move(new_elem));

      if (id_x == 0)
        boundary_info.add_side(elem_Quad, 3, SLICE_BEGIN);
      if (id_y == 0)
        boundary_info.add_side(elem_Quad, 0, SLICE_END);
      if (j < i)
        id_x++;
      if (j >= i)
        id_y--;
    }
  }
  // This loop defines the outermost layer quad elements of the central region
  for (unsigned int i = (div_num - 1) * (div_num - 1); i < div_num * div_num - 1; i++)
  {
    std::unique_ptr<Elem> new_elem;
    if (quad_elem_type == QUAD_ELEM_TYPE::QUAD4)
    {
      new_elem = std::make_unique<Quad4>();
      new_elem->set_node(0) = mesh.node_ptr(i);
      new_elem->set_node(3) = mesh.node_ptr(i + 2 * div_num - 1);
      new_elem->set_node(2) = mesh.node_ptr(i + 2 * div_num);
      new_elem->set_node(1) = mesh.node_ptr(i + 1);
    }
    else // QUAD8/QUAD9
    {
      new_elem = std::make_unique<Quad8>();
      if (quad_elem_type == QUAD_ELEM_TYPE::QUAD9)
      {
        new_elem = std::make_unique<Quad9>();
        new_elem->set_node(8) =
            mesh.node_ptr((div_num - 1) * (div_num - 1) * 4 +
                          (i - (div_num - 1) * (div_num - 1)) * 2 + 1 + ((div_num - 1) * 4 + 1));
      }
      new_elem->set_node(0) = mesh.node_ptr((div_num - 1) * (div_num - 1) * 4 +
                                            (i - (div_num - 1) * (div_num - 1)) * 2);
      new_elem->set_node(3) =
          mesh.node_ptr((div_num - 1) * (div_num - 1) * 4 +
                        (i - (div_num - 1) * (div_num - 1)) * 2 + ((div_num - 1) * 4 + 1) * 2);
      new_elem->set_node(2) =
          mesh.node_ptr((div_num - 1) * (div_num - 1) * 4 +
                        (i - (div_num - 1) * (div_num - 1)) * 2 + 2 + ((div_num - 1) * 4 + 1) * 2);
      new_elem->set_node(1) = mesh.node_ptr((div_num - 1) * (div_num - 1) * 4 +
                                            (i - (div_num - 1) * (div_num - 1)) * 2 + 2);
      new_elem->set_node(4) = mesh.node_ptr((div_num - 1) * (div_num - 1) * 4 +
                                            (i - (div_num - 1) * (div_num - 1)) * 2 + 1);
      new_elem->set_node(5) =
          mesh.node_ptr((div_num - 1) * (div_num - 1) * 4 +
                        (i - (div_num - 1) * (div_num - 1)) * 2 + 2 + ((div_num - 1) * 4 + 1));
      new_elem->set_node(6) =
          mesh.node_ptr((div_num - 1) * (div_num - 1) * 4 +
                        (i - (div_num - 1) * (div_num - 1)) * 2 + 1 + ((div_num - 1) * 4 + 1) * 2);
      new_elem->set_node(7) =
          mesh.node_ptr((div_num - 1) * (div_num - 1) * 4 +
                        (i - (div_num - 1) * (div_num - 1)) * 2 + ((div_num - 1) * 4 + 1));
    }

    Elem * elem_Quad = mesh.add_elem(std::move(new_elem));
    elem_Quad->subdomain_id() = 1 + block_id_shift;
    if (create_outward_interface_boundaries)
      boundary_info.add_side(elem_Quad, 2, 1 + boundary_id_shift);
    if (i == (div_num - 1) * (div_num - 1))
      boundary_info.add_side(elem_Quad, 3, SLICE_BEGIN);
    if (i == div_num * div_num - 2)
      boundary_info.add_side(elem_Quad, 1, SLICE_END);
    if (assign_external_boundary)
    {
      boundary_info.add_side(elem_Quad, 2, OUTER_SIDESET_ID);
      if (generate_side_specific_boundaries)
        boundary_info.add_side(
            elem_Quad,
            2,
            (i < div_num * (div_num - 1) ? OUTER_SIDESET_ID : OUTER_SIDESET_ID_ALT) + side_index);
    }
  }
}

void
PolygonMeshGeneratorBase::cenTriElemDef(ReplicatedMesh & mesh,
                                        const unsigned int num_sectors_per_side,
                                        const std::vector<Real> azimuthal_tangent,
                                        const subdomain_id_type block_id_shift,
                                        const bool create_outward_interface_boundaries,
                                        const boundary_id_type boundary_id_shift,
                                        const bool assign_external_boundary,
                                        const unsigned int side_index,
                                        const bool generate_side_specific_boundaries,
                                        const TRI_ELEM_TYPE tri_elem_type) const
{
  const unsigned short order = tri_elem_type == TRI_ELEM_TYPE::TRI3 ? 1 : 2;
  unsigned int angle_number = azimuthal_tangent.size() == 0
                                  ? num_sectors_per_side
                                  : ((azimuthal_tangent.size() - 1) / order);

  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  for (unsigned int i = 1; i <= angle_number; i++)
  {
    std::unique_ptr<Elem> new_elem;
    if (tri_elem_type == TRI_ELEM_TYPE::TRI3)
    {
      new_elem = std::make_unique<Tri3>();
      new_elem->set_node(0) = mesh.node_ptr(0);
      new_elem->set_node(2) = mesh.node_ptr(i);
      new_elem->set_node(1) = mesh.node_ptr(i + 1);
    }
    else // TRI6/TRI7
    {
      new_elem = std::make_unique<Tri6>();
      if (tri_elem_type == TRI_ELEM_TYPE::TRI7)
      {
        new_elem = std::make_unique<Tri7>();
        new_elem->set_node(6) = mesh.node_ptr(i * 2);
      }
      new_elem->set_node(0) = mesh.node_ptr(0);
      new_elem->set_node(2) = mesh.node_ptr(i * 2 + angle_number * order);
      new_elem->set_node(1) = mesh.node_ptr((i + 1) * 2 + angle_number * order);
      new_elem->set_node(3) = mesh.node_ptr(i * 2 + 1);
      new_elem->set_node(5) = mesh.node_ptr(i * 2 - 1);
      new_elem->set_node(4) = mesh.node_ptr(i * 2 + 1 + angle_number * order);
    }

    Elem * elem = mesh.add_elem(std::move(new_elem));
    if (create_outward_interface_boundaries)
      boundary_info.add_side(elem, 1, 1 + boundary_id_shift);
    elem->subdomain_id() = 1 + block_id_shift;
    if (i == 1)
      boundary_info.add_side(elem, 2, SLICE_BEGIN);
    if (i == angle_number)
      boundary_info.add_side(elem, 0, SLICE_END);
    if (assign_external_boundary)
    {
      boundary_info.add_side(elem, 1, OUTER_SIDESET_ID);
      if (generate_side_specific_boundaries)
        boundary_info.add_side(elem,
                               1,
                               (i <= angle_number / 2 ? OUTER_SIDESET_ID : OUTER_SIDESET_ID_ALT) +
                                   side_index);
    }
  }
}

void
PolygonMeshGeneratorBase::quadElemDef(ReplicatedMesh & mesh,
                                      const unsigned int num_sectors_per_side,
                                      const std::vector<unsigned int> subdomain_rings,
                                      const unsigned int side_index,
                                      const std::vector<Real> azimuthal_tangent,
                                      const subdomain_id_type block_id_shift,
                                      const dof_id_type nodeid_shift,
                                      const bool create_inward_interface_boundaries,
                                      const bool create_outward_interface_boundaries,
                                      const boundary_id_type boundary_id_shift,
                                      const bool generate_side_specific_boundaries,
                                      const QUAD_ELEM_TYPE quad_elem_type) const
{
  const unsigned short order = quad_elem_type == QUAD_ELEM_TYPE::QUAD4 ? 1 : 2;
  unsigned int angle_number = azimuthal_tangent.size() == 0
                                  ? num_sectors_per_side
                                  : ((azimuthal_tangent.size() - 1) / order);

  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  unsigned int j = 0;
  for (unsigned int k = 0; k < (subdomain_rings.size()); k++)
  {
    for (unsigned int m = 0; m < subdomain_rings[k]; m++)
    {
      for (unsigned int i = 1; i <= angle_number; i++)
      {
        std::unique_ptr<Elem> new_elem;
        if (quad_elem_type == QUAD_ELEM_TYPE::QUAD4)
        {
          new_elem = std::make_unique<Quad4>();
          new_elem->set_node(0) = mesh.node_ptr(nodeid_shift + i + (angle_number + 1) * j);
          new_elem->set_node(1) = mesh.node_ptr(nodeid_shift + i + 1 + (angle_number + 1) * j);
          new_elem->set_node(2) =
              mesh.node_ptr(nodeid_shift + i + (angle_number + 1) * (j + 1) + 1);
          new_elem->set_node(3) = mesh.node_ptr(nodeid_shift + i + (angle_number + 1) * (j + 1));
        }
        else // QUAD8/QUAD9
        {
          new_elem = std::make_unique<Quad8>();
          if (quad_elem_type == QUAD_ELEM_TYPE::QUAD9)
          {
            new_elem = std::make_unique<Quad9>();
            new_elem->set_node(8) =
                mesh.node_ptr(nodeid_shift + i * 2 + (angle_number * 2 + 1) * (j * 2 + 2));
          }
          new_elem->set_node(0) =
              mesh.node_ptr(nodeid_shift + (i - 1) * 2 + 1 + (angle_number * 2 + 1) * (j * 2 + 1));
          new_elem->set_node(1) =
              mesh.node_ptr(nodeid_shift + i * 2 + 1 + (angle_number * 2 + 1) * (j * 2 + 1));
          new_elem->set_node(2) =
              mesh.node_ptr(nodeid_shift + i * 2 + 1 + (angle_number * 2 + 1) * (j * 2 + 3));
          new_elem->set_node(3) =
              mesh.node_ptr(nodeid_shift + (i - 1) * 2 + 1 + (angle_number * 2 + 1) * (j * 2 + 3));
          new_elem->set_node(4) =
              mesh.node_ptr(nodeid_shift + i * 2 + (angle_number * 2 + 1) * (j * 2 + 1));
          new_elem->set_node(5) =
              mesh.node_ptr(nodeid_shift + i * 2 + 1 + (angle_number * 2 + 1) * (j * 2 + 2));
          new_elem->set_node(6) =
              mesh.node_ptr(nodeid_shift + i * 2 + (angle_number * 2 + 1) * (j * 2 + 3));
          new_elem->set_node(7) =
              mesh.node_ptr(nodeid_shift + (i - 1) * 2 + 1 + (angle_number * 2 + 1) * (j * 2 + 2));
        }
        Elem * elem = mesh.add_elem(std::move(new_elem));
        if (i == 1)
          boundary_info.add_side(elem, 3, SLICE_BEGIN);
        if (i == angle_number)
          boundary_info.add_side(elem, 1, SLICE_END);

        if (subdomain_rings[0] == 0)
          elem->subdomain_id() = k + 1 + block_id_shift;
        else
          elem->subdomain_id() = k + 2 + block_id_shift;

        if (m == 0 && create_inward_interface_boundaries && k > 0)
          boundary_info.add_side(elem, 0, k * 2 + boundary_id_shift);
        if (m == (subdomain_rings[k] - 1))
        {
          if (k == (subdomain_rings.size() - 1))
          {
            boundary_info.add_side(elem, 2, OUTER_SIDESET_ID);
            if (generate_side_specific_boundaries)
            {
              if (i <= angle_number / 2)
                boundary_info.add_side(elem, 2, OUTER_SIDESET_ID + side_index);
              else
                boundary_info.add_side(elem, 2, OUTER_SIDESET_ID_ALT + side_index);
            }
          }
          else if (create_outward_interface_boundaries)
            boundary_info.add_side(elem, 2, k * 2 + 1 + boundary_id_shift);
        }
      }
      j++;
    }
  }
}

std::unique_ptr<ReplicatedMesh>
PolygonMeshGeneratorBase::buildSimplePeripheral(
    const unsigned int num_sectors_per_side,
    const unsigned int peripheral_invervals,
    const std::vector<std::pair<Real, Real>> & positions_inner,
    const std::vector<std::pair<Real, Real>> & d_positions_outer,
    const subdomain_id_type id_shift,
    const QUAD_ELEM_TYPE quad_elem_type,
    const bool create_inward_interface_boundaries,
    const bool create_outward_interface_boundaries)
{
  auto mesh = buildReplicatedMesh(2);
  std::pair<Real, Real> positions_p;

  // generate node positions
  for (unsigned int i = 0; i <= peripheral_invervals; i++)
  {
    for (unsigned int j = 0; j <= num_sectors_per_side / 2; j++)
    {
      positions_p = pointInterpolate(positions_inner[0].first,
                                     positions_inner[0].second,
                                     d_positions_outer[0].first,
                                     d_positions_outer[0].second,
                                     positions_inner[1].first,
                                     positions_inner[1].second,
                                     d_positions_outer[1].first,
                                     d_positions_outer[1].second,
                                     i,
                                     j,
                                     num_sectors_per_side,
                                     peripheral_invervals);
      mesh->add_point(Point(positions_p.first, positions_p.second, 0.0));
    }
    for (unsigned int j = 1; j <= num_sectors_per_side / 2; j++)
    {
      positions_p = pointInterpolate(positions_inner[1].first,
                                     positions_inner[1].second,
                                     d_positions_outer[1].first,
                                     d_positions_outer[1].second,
                                     positions_inner[2].first,
                                     positions_inner[2].second,
                                     d_positions_outer[2].first,
                                     d_positions_outer[2].second,
                                     i,
                                     j,
                                     num_sectors_per_side,
                                     peripheral_invervals);
      mesh->add_point(Point(positions_p.first, positions_p.second, 0.0));
    }
  }

  // element definition
  BoundaryInfo & boundary_info = mesh->get_boundary_info();

  for (unsigned int i = 0; i < peripheral_invervals; i++)
  {
    for (unsigned int j = 0; j < num_sectors_per_side; j++)
    {
      std::unique_ptr<Elem> new_elem;

      new_elem = std::make_unique<Quad4>();
      new_elem->set_node(0) = mesh->node_ptr(j + (num_sectors_per_side + 1) * (i));
      new_elem->set_node(1) = mesh->node_ptr(j + 1 + (num_sectors_per_side + 1) * (i));
      new_elem->set_node(2) = mesh->node_ptr(j + 1 + (num_sectors_per_side + 1) * (i + 1));
      new_elem->set_node(3) = mesh->node_ptr(j + (num_sectors_per_side + 1) * (i + 1));

      Elem * elem = mesh->add_elem(std::move(new_elem));

      // add subdoamin and boundary IDs
      elem->subdomain_id() = PERIPHERAL_ID_SHIFT + id_shift;
      if (i == 0)
      {
        boundary_info.add_side(elem, 0, OUTER_SIDESET_ID);
        if (create_inward_interface_boundaries)
          boundary_info.add_side(elem, 0, SLICE_ALT + id_shift * 2);
      }
      if (i == peripheral_invervals - 1)
      {
        boundary_info.add_side(elem, 2, OUTER_SIDESET_ID);
        if (create_outward_interface_boundaries)
          boundary_info.add_side(elem, 2, SLICE_ALT + id_shift * 2 + 1);
      }
      if (j == 0)
        boundary_info.add_side(elem, 3, OUTER_SIDESET_ID);
      if (j == num_sectors_per_side - 1)
        boundary_info.add_side(elem, 1, OUTER_SIDESET_ID);
    }
  }

  // convert element to second order if needed
  if (quad_elem_type != QUAD_ELEM_TYPE::QUAD4)
  {
    // full_ordered 2nd order element --> QUAD9, otherwise QUAD8
    const bool full_ordered = (quad_elem_type == QUAD_ELEM_TYPE::QUAD9);
    mesh->all_second_order(full_ordered);
  }

  return mesh;
}

void
PolygonMeshGeneratorBase::adjustPeripheralQuadraticElements(
    MeshBase & out_mesh, const QUAD_ELEM_TYPE boundary_quad_elem_type) const
{
  const auto side_list = out_mesh.get_boundary_info().build_side_list();

  // select out elements on outer boundary
  // std::set used to filter duplicate elem_ids
  std::set<dof_id_type> elem_set;
  for (auto side_item : side_list)
  {
    boundary_id_type boundary_id = std::get<2>(side_item);
    dof_id_type elem_id = std::get<0>(side_item);

    if (boundary_id == OUTER_SIDESET_ID)
      elem_set.insert(elem_id);
  }

  // adjust nodes for outer boundary elements
  for (const auto elem_id : elem_set)
  {
    Elem * elem = out_mesh.elem_ptr(elem_id);

    // adjust right side mid-edge node
    Point pt_5 = (elem->point(1) + elem->point(2)) / 2.0;
    out_mesh.add_point(pt_5, elem->node_ptr(5)->id());

    // adjust left side mid-edge node
    Point pt_7 = (elem->point(0) + elem->point(3)) / 2.0;
    out_mesh.add_point(pt_7, elem->node_ptr(7)->id());

    // adjust central node when using QUAD9
    if (boundary_quad_elem_type == QUAD_ELEM_TYPE::QUAD9)
    {
      Point pt_8 = elem->true_centroid();
      out_mesh.add_point(pt_8, elem->node_ptr(8)->id());
    }
  }
}

std::pair<Real, Real>
PolygonMeshGeneratorBase::pointInterpolate(const Real pi_1_x,
                                           const Real pi_1_y,
                                           const Real d_po_1_x,
                                           const Real d_po_1_y,
                                           const Real pi_2_x,
                                           const Real pi_2_y,
                                           const Real d_po_2_x,
                                           const Real d_po_2_y,
                                           const unsigned int i,
                                           const unsigned int j,
                                           const unsigned int num_sectors_per_side,
                                           const unsigned int peripheral_intervals) const
{
  auto position_px_inner =
      (pi_1_x * (num_sectors_per_side / 2.0 - j) + pi_2_x * j) / (num_sectors_per_side / 2.0);
  auto position_py_inner =
      (pi_1_y * (num_sectors_per_side / 2.0 - j) + pi_2_y * j) / (num_sectors_per_side / 2.0);
  auto position_px_outer =
      (d_po_1_x * (num_sectors_per_side / 2.0 - j) + d_po_2_x * j) / (num_sectors_per_side / 2.0);
  auto position_py_outer =
      (d_po_1_y * (num_sectors_per_side / 2.0 - j) + d_po_2_y * j) / (num_sectors_per_side / 2.0);
  auto position_px = position_px_inner + position_px_outer * i / peripheral_intervals;
  auto position_py = position_py_inner + position_py_outer * i / peripheral_intervals;
  return std::make_pair(position_px, position_py);
}

void
PolygonMeshGeneratorBase::nodeCoordRotate(Real & x, Real & y, const Real theta) const
{
  const Real x_tmp = x;
  const Real y_tmp = y;
  x = x_tmp * std::cos(theta * M_PI / 180.0) - y_tmp * std::sin(theta * M_PI / 180.0);
  y = x_tmp * std::sin(theta * M_PI / 180.0) + y_tmp * std::cos(theta * M_PI / 180.0);
}

void
PolygonMeshGeneratorBase::cutOffPolyDeform(MeshBase & mesh,
                                           const Real orientation,
                                           const Real y_max_0,
                                           const Real y_max_n,
                                           const Real y_min,
                                           const unsigned int mesh_type,
                                           const Real unit_angle,
                                           const Real tols) const
{
  for (auto & node_ptr : as_range(mesh.nodes_begin(), mesh.nodes_end()))
  {
    // This function can definitely be optimized in future for better efficiency.
    Real & x = (*node_ptr)(0);
    Real & y = (*node_ptr)(1);
    if (mesh_type == CORNER_MESH)
    {
      nodeCoordRotate(x, y, orientation);
      if (x >= 0.0 && y > y_max_0)
        y = y - y_max_0 + y_max_n;
      else if (x >= 0.0 && y >= y_min)
        y = (y - y_min) / (y_max_0 - y_min) * (y_max_n - y_min) + y_min;
      else if (y > -x / std::tan(unit_angle / 360.0 * M_PI) + tols && y > y_max_0)
      {
        x /= y;
        y = y - y_max_0 + y_max_n;
        x *= y;
      }
      else if (y > -x / std::tan(unit_angle / 360.0 * M_PI) + tols && y >= y_min)
      {
        x /= y;
        y = (y - y_min) / (y_max_0 - y_min) * (y_max_n - y_min) + y_min;
        x *= y;
      }
      nodeCoordRotate(x, y, -orientation);

      nodeCoordRotate(x, y, orientation - unit_angle);
      if (x <= 0 && y > y_max_0)
        y = y - y_max_0 + y_max_n;
      else if (x <= 0 && y >= y_min)
        y = (y - y_min) / (y_max_0 - y_min) * (y_max_n - y_min) + y_min;
      else if (y >= x / std::tan(unit_angle / 360.0 * M_PI) - tols && y > y_max_0)
      {
        x /= y;
        y = y - y_max_0 + y_max_n;
        x *= y;
      }
      else if (y >= x / std::tan(unit_angle / 360.0 * M_PI) - tols && y >= y_min)
      {
        x /= y;
        y = (y - y_min) / (y_max_0 - y_min) * (y_max_n - y_min) + y_min;
        x *= y;
      }
      nodeCoordRotate(x, y, unit_angle - orientation);
    }
    else
    {
      nodeCoordRotate(x, y, orientation);
      if (y > y_max_0)
        y = y - y_max_0 + y_max_n;
      else if (y >= y_min)
        y = (y - y_min) / (y_max_0 - y_min) * (y_max_n - y_min) + y_min;
      nodeCoordRotate(x, y, -orientation);
    }
  }
}

std::pair<Real, Real>
PolygonMeshGeneratorBase::fourPointIntercept(const std::pair<Real, Real> & p1,
                                             const std::pair<Real, Real> & p2,
                                             const std::pair<Real, Real> & p3,
                                             const std::pair<Real, Real> & p4) const
{
  const Real x1 = p1.first;
  const Real y1 = p1.second;
  const Real x2 = p2.first;
  const Real y2 = p2.second;
  const Real x3 = p3.first;
  const Real y3 = p3.second;
  const Real x4 = p4.first;
  const Real y4 = p4.second;

  Real x = -((x1 - x2) * (y3 * x4 - x3 * y4) - (x3 - x4) * (y1 * x2 - x1 * y2)) /
           ((y1 - y2) * (x3 - x4) - (y3 - y4) * (x1 - x2));
  Real y = -((y1 - y2) * (y3 * x4 - x3 * y4) - (y3 - y4) * (y1 * x2 - x1 * y2)) /
           ((y1 - y2) * (x3 - x4) - (y3 - y4) * (x1 - x2));

  return std::make_pair(x, y);
}

std::vector<Real>
PolygonMeshGeneratorBase::azimuthalAnglesCollector(ReplicatedMesh & mesh,
                                                   std::vector<Point> & boundary_points,
                                                   const Real lower_azi,
                                                   const Real upper_azi,
                                                   const unsigned int return_type,
                                                   const unsigned int num_sides,
                                                   const boundary_id_type bid,
                                                   const bool calculate_origin,
                                                   const Real input_origin_x,
                                                   const Real input_origin_y,
                                                   const Real tol) const
{
  std::vector<std::tuple<dof_id_type, unsigned short int, boundary_id_type>> side_list =
      mesh.get_boundary_info().build_side_list();
  mesh.get_boundary_info().build_node_list_from_side_list();
  std::vector<std::tuple<dof_id_type, boundary_id_type>> node_list =
      mesh.get_boundary_info().build_node_list();

  std::vector<Real> bd_x_list;
  std::vector<Real> bd_y_list;
  std::vector<Point> bd_p_list;
  Real origin_x = 0.0;
  Real origin_y = 0.0;
  Real tmp_azi;
  const Real mid_azi = lower_azi <= upper_azi ? (lower_azi + upper_azi) / 2.0
                                              : (lower_azi + upper_azi + 360.0) / 2.0;
  for (unsigned int i = 0; i < node_list.size(); ++i)
    if (std::get<1>(node_list[i]) == bid)
    {
      bd_x_list.push_back((mesh.node_ref(std::get<0>(node_list[i])))(0));
      bd_y_list.push_back((mesh.node_ref(std::get<0>(node_list[i])))(1));
      bd_p_list.push_back((mesh.node_ref(std::get<0>(node_list[i]))));
    }

  if (calculate_origin)
  {
    const Point origin_pt = MooseMeshUtils::meshCentroidCalculator(mesh);
    origin_x = origin_pt(0);
    origin_y = origin_pt(1);
  }
  else
  {
    origin_x = input_origin_x;
    origin_y = input_origin_y;
  }

  std::vector<std::pair<Real, Point>> azi_point_pairs;

  for (unsigned int i = 0; i < bd_x_list.size(); ++i)
  {
    tmp_azi = atan2(bd_y_list[i] - origin_y, bd_x_list[i] - origin_x) * 180.0 / M_PI;
    if ((lower_azi <= upper_azi && (tmp_azi >= lower_azi - tol && tmp_azi <= upper_azi + tol)) ||
        (lower_azi > upper_azi && (tmp_azi >= lower_azi - tol || tmp_azi <= upper_azi + tol)))
    {
      azi_point_pairs.push_back(
          std::make_pair(return_type == ANGLE_DEGREE
                             ? (tmp_azi - mid_azi)
                             : (1.0 + std::cos(M_PI / num_sides) / std::sin(M_PI / num_sides) *
                                          std::tan((tmp_azi - mid_azi) / 180.0 * M_PI)),
                         bd_p_list[i]));
    }
  }
  std::sort(azi_point_pairs.begin(), azi_point_pairs.end());

  std::vector<Real> azimuthal_output;
  for (auto it = std::make_move_iterator(azi_point_pairs.begin()),
            end = std::make_move_iterator(azi_point_pairs.end());
       it != end;
       it++)
  {
    azimuthal_output.push_back(std::move(it->first));
    boundary_points.push_back(std::move(it->second));
  }

  return azimuthal_output;
}

std::vector<Real>
PolygonMeshGeneratorBase::azimuthalAnglesCollector(ReplicatedMesh & mesh,
                                                   const Real lower_azi,
                                                   const Real upper_azi,
                                                   const unsigned int return_type,
                                                   const unsigned int num_sides,
                                                   const boundary_id_type bid,
                                                   const bool calculate_origin,
                                                   const Real input_origin_x,
                                                   const Real input_origin_y,
                                                   const Real tol) const
{
  std::vector<Point> boundary_points;
  return azimuthalAnglesCollector(mesh,
                                  boundary_points,
                                  lower_azi,
                                  upper_azi,
                                  return_type,
                                  num_sides,
                                  bid,
                                  calculate_origin,
                                  input_origin_x,
                                  input_origin_y,
                                  tol);
}

std::vector<std::vector<Real>>
PolygonMeshGeneratorBase::biasTermsCalculator(
    const std::vector<Real> radial_biases,
    const std::vector<unsigned int> intervals,
    const multiBdryLayerParams inner_boundary_layer_params,
    const multiBdryLayerParams outer_boundary_layer_params) const
{
  std::vector<std::vector<Real>> bias_terms_vec;
  for (unsigned int i = 0; i < radial_biases.size(); i++)
    bias_terms_vec.push_back(biasTermsCalculator(radial_biases[i],
                                                 intervals[i],
                                                 {0.0,
                                                  inner_boundary_layer_params.fractions[i],
                                                  inner_boundary_layer_params.intervals[i],
                                                  inner_boundary_layer_params.biases[i]},
                                                 {0.0,
                                                  outer_boundary_layer_params.fractions[i],
                                                  outer_boundary_layer_params.intervals[i],
                                                  outer_boundary_layer_params.biases[i]}));
  return bias_terms_vec;
}

std::vector<Real>
PolygonMeshGeneratorBase::biasTermsCalculator(
    const Real radial_bias,
    const unsigned int intervals,
    const singleBdryLayerParams inner_boundary_layer_params,
    const singleBdryLayerParams outer_boundary_layer_params) const
{
  // To get biased indices:
  // If no bias is involved, namely bias factor = 1.0, the increment in indices is uniform.
  // Thus, (i + 1) is used to get such linearly increasing indices.
  // If a non-trivial bias factor q is used, the increment in the indices is geometric
  // progression. So, if first (i = 0) increment is 1.0, second (i = 1) is q, third (i = 2) is
  // q^2,..., last or n_interval'th is q^(n_interval - 1). Then, the summation of the first (i +
  // 1) increments over the summation of all n_interval increments is the (i + 1)th index The
  // summation of the first (i + 1) increments is (1.0 - q^(i + 1)) / (1 - q); The summation of
  // all n_interval increments is (1.0 - q^n_interval) / (1 - q); Thus, the index is (1.0 - q^(i +
  // 1)) / (1.0 - q^n_interval)
  // This approach is used by inner boundary layer, main region, outer boundary layer separately.

  std::vector<Real> biased_terms;
  for (unsigned int i = 0; i < inner_boundary_layer_params.intervals; i++)
    biased_terms.push_back(
        MooseUtils::absoluteFuzzyEqual(inner_boundary_layer_params.bias, 1.0)
            ? ((Real)(i + 1) * inner_boundary_layer_params.fraction /
               (Real)inner_boundary_layer_params.intervals)
            : ((1.0 - std::pow(inner_boundary_layer_params.bias, (Real)(i + 1))) /
               (1.0 - std::pow(inner_boundary_layer_params.bias,
                               (Real)(inner_boundary_layer_params.intervals))) *
               inner_boundary_layer_params.fraction));
  for (unsigned int i = 0; i < intervals; i++)
    biased_terms.push_back(inner_boundary_layer_params.fraction +
                           (MooseUtils::absoluteFuzzyEqual(radial_bias, 1.0)
                                ? ((Real)(i + 1) *
                                   (1.0 - inner_boundary_layer_params.fraction -
                                    outer_boundary_layer_params.fraction) /
                                   (Real)intervals)
                                : ((1.0 - std::pow(radial_bias, (Real)(i + 1))) /
                                   (1.0 - std::pow(radial_bias, (Real)(intervals))) *
                                   (1.0 - inner_boundary_layer_params.fraction -
                                    outer_boundary_layer_params.fraction))));
  for (unsigned int i = 0; i < outer_boundary_layer_params.intervals; i++)
    biased_terms.push_back(
        1.0 - outer_boundary_layer_params.fraction +
        (MooseUtils::absoluteFuzzyEqual(outer_boundary_layer_params.bias, 1.0)
             ? ((Real)(i + 1) * outer_boundary_layer_params.fraction /
                (Real)outer_boundary_layer_params.intervals)
             : ((1.0 - std::pow(outer_boundary_layer_params.bias, (Real)(i + 1))) /
                (1.0 - std::pow(outer_boundary_layer_params.bias,
                                (Real)(outer_boundary_layer_params.intervals))) *
                outer_boundary_layer_params.fraction)));
  return biased_terms;
}

void
PolygonMeshGeneratorBase::addRingAndSectorIDParams(InputParameters & params)
{
  params.addParam<std::string>("sector_id_name",
                               "Name of integer (reporting) ID for sector regions to use the "
                               "reporting ID for azimuthal sector regions of ring geometry block.");
  params.addParam<std::string>("ring_id_name",
                               "Name of integer (reporting) ID for ring regions to use the "
                               "reporting ID for annular regions of ring geometry block.");
  MooseEnum ring_id_option("block_wise ring_wise", "block_wise");
  params.addParam<MooseEnum>(
      "ring_id_assign_type", ring_id_option, "Type of ring ID assignment: block_wise or ring_wise");
  params.addParamNamesToGroup("sector_id_name ring_id_name ring_id_assign_type", "Ring/Sector IDs");
}

void
PolygonMeshGeneratorBase::setSectorExtraIDs(MeshBase & mesh,
                                            const std::string id_name,
                                            const unsigned int num_sides,
                                            const std::vector<unsigned int> num_sectors_per_side)
{
  const auto extra_id_index = mesh.add_elem_integer(id_name);
  // vector to store sector ids for each element
  auto elem_it = mesh.elements_begin();
  unsigned int id = 1;
  // starting element id of the current sector
  for (unsigned int is = 0; is < num_sides; ++is)
  {
    // number of elements in the current sector
    unsigned int nelem_sector =
        mesh.n_elem() * num_sectors_per_side[is] /
        (accumulate(num_sectors_per_side.begin(), num_sectors_per_side.end(), 0));
    // assign sector ids to mesh
    for (unsigned i = 0; i < nelem_sector; ++i, ++elem_it)
      (*elem_it)->set_extra_integer(extra_id_index, id);
    // update sector id
    ++id;
  }
}

void
PolygonMeshGeneratorBase::setRingExtraIDs(MeshBase & mesh,
                                          const std::string id_name,
                                          const unsigned int num_sides,
                                          const std::vector<unsigned int> num_sectors_per_side,
                                          const std::vector<unsigned int> ring_intervals,
                                          const bool ring_wise_id,
                                          const bool quad_center_elements)
{
  // this function assumes that elements are ordered by rings (inner) then by sectors (outer
  // ordering)
  const auto extra_id_index = mesh.add_elem_integer(id_name);
  auto elem_it = mesh.elements_begin();
  for (unsigned int is = 0; is < num_sides; ++is)
  {
    // number of elements in the current sector
    unsigned int nelem = mesh.n_elem() * num_sectors_per_side[is] /
                         (accumulate(num_sectors_per_side.begin(), num_sectors_per_side.end(), 0));
    if (!ring_wise_id)
    {
      for (unsigned int ir : index_range(ring_intervals))
      {
        // number of elements in the current ring and sector
        unsigned int nelem_annular_ring = num_sectors_per_side[is] * ring_intervals[ir];
        // if _quad_center_elements is true, the number of elements in center ring are
        // _num_sectors_per_side[is] * _num_sectors_per_side[is] / 4
        if (quad_center_elements && ir == 0)
          nelem_annular_ring = num_sectors_per_side[is] * (ring_intervals[ir] - 1) +
                               num_sectors_per_side[is] * num_sectors_per_side[is] / 4;
        // assign ring id
        for (unsigned i = 0; i < nelem_annular_ring; ++i, ++elem_it)
          (*elem_it)->set_extra_integer(extra_id_index, ir + 1);
        // update number of elements in background region of current side.
        nelem -= nelem_annular_ring;
      }
    }
    else
    {
      unsigned int ir = 0;
      for (unsigned int ir0 : index_range(ring_intervals))
      {
        for (unsigned int ir1 = 0; ir1 < ring_intervals[ir0]; ++ir1)
        {
          // number of elements in the current ring and sector
          unsigned int nelem_annular_ring = num_sectors_per_side[is];
          // if _quad_center_elements is true, the number of elements in center ring are
          // _num_sectors_per_side[is] * _num_sectors_per_side[is] / 4
          if (quad_center_elements && ir == 0)
            nelem_annular_ring = num_sectors_per_side[is] * num_sectors_per_side[is] / 4;
          // assign ring id
          for (unsigned i = 0; i < nelem_annular_ring; ++i, ++elem_it)
            (*elem_it)->set_extra_integer(extra_id_index, ir + 1);
          // update ring id
          ++ir;
          // update number of elements in background region of current side.
          nelem -= nelem_annular_ring;
        }
      }
    }
    // assign ring id of 0 to the background region
    for (unsigned i = 0; i < nelem; ++i, ++elem_it)
      (*elem_it)->set_extra_integer(extra_id_index, 0);
  }
}

void
PolygonMeshGeneratorBase::reassignBoundaryIDs(MeshBase & mesh,
                                              const boundary_id_type id_shift,
                                              const std::set<boundary_id_type> & boundary_ids,
                                              const bool reverse)
{
  const std::set<boundary_id_type> existing_boundary_ids =
      mesh.get_boundary_info().get_boundary_ids();
  for (const auto id : boundary_ids)
  {

    const boundary_id_type old_id = (!reverse) ? id : id + id_shift;
    const boundary_id_type new_id = (!reverse) ? id + id_shift : id;
    auto it = existing_boundary_ids.find(old_id);
    if (it != existing_boundary_ids.end())
      MooseMesh::changeBoundaryId(mesh, old_id, new_id, true);
  }
}

std::set<boundary_id_type>
PolygonMeshGeneratorBase::getInterfaceBoundaryIDs(
    const std::vector<std::vector<unsigned int>> & pattern,
    const std::vector<std::vector<boundary_id_type>> & interface_boundary_id_shift_pattern,
    const std::set<boundary_id_type> & boundary_ids,
    const std::vector<std::set<boundary_id_type>> & input_interface_boundary_ids,
    const bool use_interface_boundary_id_shift,
    const bool create_interface_boundary_id,
    const unsigned int num_extra_layers) const
{
  std::set<boundary_id_type> interface_boundary_ids;
  // add existing interface boundary ids from input meshes
  if (use_interface_boundary_id_shift)
  {
    for (const auto i : make_range(pattern.size()))
      for (const auto j : make_range(pattern[i].size()))
      {
        const auto & ids = input_interface_boundary_ids[pattern[i][j]];
        for (const auto & id : ids)
        {
          const boundary_id_type new_id = id + interface_boundary_id_shift_pattern[i][j];
          auto it = boundary_ids.find(new_id);
          if (it != boundary_ids.end())
            interface_boundary_ids.insert(new_id);
        }
      }
  }
  else
  {
    for (const auto & ids : input_interface_boundary_ids)
      for (const auto & id : ids)
      {
        auto it = boundary_ids.find(id);
        if (it != boundary_ids.end())
          interface_boundary_ids.insert(id);
      }
  }
  // add unshifted interface boundary ids for the duct & background regions
  if (create_interface_boundary_id)
    for (const auto i : make_range(num_extra_layers))
    {
      boundary_id_type id = SLICE_ALT + i * 2 + 1;
      auto it = boundary_ids.find(id);
      if (it != boundary_ids.end())
        interface_boundary_ids.insert(id);
      id = SLICE_ALT + i * 2;
      it = boundary_ids.find(id);
      if (it != boundary_ids.end())
        interface_boundary_ids.insert(id);
    }
  return interface_boundary_ids;
}

PolygonMeshGeneratorBase::multiBdryLayerParams
PolygonMeshGeneratorBase::modifiedMultiBdryLayerParamsCreator(
    const multiBdryLayerParams & original_multi_bdry_layer_params, const unsigned int order) const
{
  multiBdryLayerParams mod_multi_bdry_layer_params(original_multi_bdry_layer_params);
  std::for_each(mod_multi_bdry_layer_params.intervals.begin(),
                mod_multi_bdry_layer_params.intervals.end(),
                [&order](unsigned int & n) { n *= order; });
  std::for_each(mod_multi_bdry_layer_params.biases.begin(),
                mod_multi_bdry_layer_params.biases.end(),
                [&order](Real & n) { n = std::pow(n, 1.0 / order); });
  return mod_multi_bdry_layer_params;
}

PolygonMeshGeneratorBase::singleBdryLayerParams
PolygonMeshGeneratorBase::modifiedSingleBdryLayerParamsCreator(
    const singleBdryLayerParams & original_single_bdry_layer_params, const unsigned int order) const
{
  singleBdryLayerParams mod_single_bdry_layer_params(original_single_bdry_layer_params);
  mod_single_bdry_layer_params.intervals *= order;
  mod_single_bdry_layer_params.bias = std::pow(mod_single_bdry_layer_params.bias, 1.0 / order);
  return mod_single_bdry_layer_params;
}

std::string
PolygonMeshGeneratorBase::pitchMetaDataErrorGenerator(
    const std::vector<MeshGeneratorName> & input_names,
    const std::vector<Real> & metadata_vals,
    const std::string & metadata_name) const
{
  FormattedTable table;
  for (unsigned int i = 0; i < input_names.size(); i++)
  {
    table.addRow(i);
    table.addData<std::string>("input name", (std::string)input_names[i]);
    table.addData<Real>(metadata_name, metadata_vals[i]);
  }
  table.outputTimeColumn(false);
  std::stringstream detailed_error;
  table.printTable(detailed_error);
  return "\n" + detailed_error.str();
}
