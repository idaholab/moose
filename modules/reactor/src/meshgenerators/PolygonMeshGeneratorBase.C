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

#include <cmath>

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
    const bool generate_side_specific_boundaries)
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
                    generate_side_specific_boundaries);
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
    const bool generate_side_specific_boundaries)
{
  bool has_rings(ring_radii.size());
  bool has_ducts(ducts_center_dist.size());
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

  unsigned int angle_number =
      azimuthal_tangent.size() == 0 ? num_sectors_per_side : (azimuthal_tangent.size() - 1);

  // Geometries
  const Real corner_to_corner =
      pitch / std::cos(M_PI / virtual_side_number); // distance of bin center to cell corner
  const Real corner_p[2][2] = {
      {0.0, 0.5 * corner_to_corner},
      {0.5 * corner_to_corner * pitch_scale_factor * std::sin(2.0 * M_PI / virtual_side_number),
       0.5 * corner_to_corner * pitch_scale_factor * std::cos(2.0 * M_PI / virtual_side_number)}};
  const unsigned int div_num = angle_number / 2 + 1;
  std::vector<std::vector<Node *>> nodes(div_num, std::vector<Node *>(div_num));
  if (quad_center_elements)
  {
    Real ring_radii_0;

    if (has_rings)
      ring_radii_0 = ring_radii.front() * rings_bias_terms.front().front();
    else if (has_ducts)
      ring_radii_0 = ducts_center_dist.front() * std::cos(M_PI / virtual_side_number) *
                     main_background_bias_terms.front();
    else
      ring_radii_0 = pitch / 2.0 * main_background_bias_terms.front();
    // If center_quad_factor is zero, default value (div_num - 1)/div_num  is used.
    ring_radii_0 *=
        center_quad_factor == 0.0 ? (((Real)div_num - 1.0) / (Real)div_num) : center_quad_factor;

    centerNodes(*mesh, virtual_side_number, div_num, ring_radii_0, nodes);
  }
  else // pin-cell center
    mesh->add_point(Point(0.0, 0.0, 0.0));

  // create nodes for the ring regions
  if (has_rings)
    ringNodes(*mesh,
              ring_radii,
              total_ring_layers,
              rings_bias_terms,
              num_sectors_per_side,
              corner_p,
              corner_to_corner,
              azimuthal_tangent);

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
    background_out = ducts_center_dist.front();
    background_corner_distance =
        ducts_center_dist.front(); // it is the center to duct (innermost duct) corner distance
  }
  else
  {
    background_out = 0.5 * corner_to_corner;
    background_corner_distance = 0.5 * corner_to_corner; // it is the center to hex corner distance
  }

  background_corner_radial_interval_length =
      (background_out - background_in) / background_intervals;

  node_id_background_meta = mesh->n_nodes();

  // create nodes for background region
  backgroundNodes(*mesh,
                  num_sectors_per_side,
                  background_intervals,
                  main_background_bias_terms,
                  background_corner_distance,
                  background_corner_radial_interval_length,
                  corner_p,
                  corner_to_corner,
                  background_in,
                  azimuthal_tangent);

  // create nodes for duct regions
  if (has_ducts)
    ductNodes(*mesh,
              &ducts_center_dist,
              total_ducts_layers,
              duct_bias_terms,
              num_sectors_per_side,
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
    is_central_region_independent = background_inner_boundary_layer_params.intervals +
                                        background_intervals +
                                        background_outer_boundary_layer_params.intervals ==
                                    1;
  else
    is_central_region_independent = ring_layers[0] + ring_inner_boundary_layer_params.intervals[0] +
                                        ring_outer_boundary_layer_params.intervals[0] ==
                                    1;

  // Assign elements, boundaries, and subdomains;
  // Add Tri3 or Quad4 mesh into innermost (central) region
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
                   generate_side_specific_boundaries);
  else
    cenTriElemDef(*mesh,
                  num_sectors_per_side,
                  azimuthal_tangent,
                  block_id_shift,
                  create_outward_interface_boundaries && is_central_region_independent,
                  boundary_id_shift,
                  (!has_rings) && (!has_ducts) && (background_intervals == 1),
                  side_index,
                  generate_side_specific_boundaries);

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
    else
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
              quad_center_elements ? (div_num * div_num - 1) : 0,
              create_inward_interface_boundaries,
              create_outward_interface_boundaries,
              boundary_id_shift,
              generate_side_specific_boundaries);

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
                                         const bool generate_side_specific_boundaries) const
{

  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  // This loop defines quad elements for the central regions except for the outermost layer
  for (unsigned int i = 0; i < div_num - 1; i++)
  {
    unsigned int id_x = 0;
    unsigned int id_y = i;
    for (unsigned int j = 0; j < 2 * i + 1; j++)
    {
      Elem * elem_Quad4 = mesh.add_elem(new Quad4);
      elem_Quad4->set_node(0) = nodes[id_x][id_y];
      elem_Quad4->set_node(3) = nodes[id_x][id_y + 1];
      elem_Quad4->set_node(2) = nodes[id_x + 1][id_y + 1];
      elem_Quad4->set_node(1) = nodes[id_x + 1][id_y];
      elem_Quad4->subdomain_id() = 1 + block_id_shift;
      if (id_x == 0)
        boundary_info.add_side(elem_Quad4, 3, SLICE_BEGIN);
      if (id_y == 0)
        boundary_info.add_side(elem_Quad4, 0, SLICE_END);
      if (j < i)
        id_x++;
      if (j >= i)
        id_y--;
    }
  }
  // This loop defines the outermost layer quad elements of the central region
  for (unsigned int i = (div_num - 1) * (div_num - 1); i < div_num * div_num - 1; i++)
  {
    Elem * elem_Quad4 = mesh.add_elem(new Quad4);
    elem_Quad4->set_node(0) = mesh.node_ptr(i);
    elem_Quad4->set_node(3) = mesh.node_ptr(i + 2 * div_num - 1);
    elem_Quad4->set_node(2) = mesh.node_ptr(i + 2 * div_num);
    elem_Quad4->set_node(1) = mesh.node_ptr(i + 1);
    elem_Quad4->subdomain_id() = 1 + block_id_shift;
    if (create_outward_interface_boundaries)
      boundary_info.add_side(elem_Quad4, 2, 1 + boundary_id_shift);
    if (i == (div_num - 1) * (div_num - 1))
      boundary_info.add_side(elem_Quad4, 3, SLICE_BEGIN);
    if (i == div_num * div_num - 2)
      boundary_info.add_side(elem_Quad4, 1, SLICE_END);
    if (assign_external_boundary)
    {
      boundary_info.add_side(elem_Quad4, 2, OUTER_SIDESET_ID);
      if (generate_side_specific_boundaries)
        boundary_info.add_side(
            elem_Quad4,
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
                                        const bool generate_side_specific_boundaries) const
{
  unsigned int angle_number =
      azimuthal_tangent.size() == 0 ? num_sectors_per_side : (azimuthal_tangent.size() - 1);

  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  for (unsigned int i = 1; i <= angle_number; i++)
  {
    Elem * elem = mesh.add_elem(new Tri3);
    elem->set_node(0) = mesh.node_ptr(0);
    elem->set_node(2) = mesh.node_ptr(i);
    elem->set_node(1) = mesh.node_ptr(i + 1);
    if (create_outward_interface_boundaries)
      boundary_info.add_side(elem, 1, 1 + boundary_id_shift);
    elem->subdomain_id() = 1 + block_id_shift;
    if (i == 1)
      boundary_info.add_side(elem, 2, SLICE_BEGIN);
    else if (i == angle_number)
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
                                      const bool generate_side_specific_boundaries) const
{
  unsigned int angle_number =
      azimuthal_tangent.size() == 0 ? num_sectors_per_side : (azimuthal_tangent.size() - 1);

  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  unsigned int j = 0;
  for (unsigned int k = 0; k < (subdomain_rings.size()); k++)
  {
    for (unsigned int m = 0; m < subdomain_rings[k]; m++)
    {
      for (unsigned int i = 1; i <= angle_number; i++)
      {

        Elem * elem_Quad4 = mesh.add_elem(new Quad4);
        elem_Quad4->set_node(0) = mesh.node_ptr(nodeid_shift + i + (angle_number + 1) * j);
        elem_Quad4->set_node(1) = mesh.node_ptr(nodeid_shift + i + 1 + (angle_number + 1) * j);
        elem_Quad4->set_node(2) =
            mesh.node_ptr(nodeid_shift + i + (angle_number + 1) * (j + 1) + 1);
        elem_Quad4->set_node(3) = mesh.node_ptr(nodeid_shift + i + (angle_number + 1) * (j + 1));
        if (i == 1)
          boundary_info.add_side(elem_Quad4, 3, SLICE_BEGIN);
        else if (i == angle_number)
          boundary_info.add_side(elem_Quad4, 1, SLICE_END);

        if (subdomain_rings[0] == 0)
          elem_Quad4->subdomain_id() = k + 1 + block_id_shift;
        else
          elem_Quad4->subdomain_id() = k + 2 + block_id_shift;

        if (m == 0 && create_inward_interface_boundaries && k > 0)
          boundary_info.add_side(elem_Quad4, 0, k * 2 + boundary_id_shift);
        if (m == (subdomain_rings[k] - 1))
        {
          if (k == (subdomain_rings.size() - 1))
          {
            boundary_info.add_side(elem_Quad4, 2, OUTER_SIDESET_ID);
            if (generate_side_specific_boundaries)
            {
              if (i <= angle_number / 2)
                boundary_info.add_side(elem_Quad4, 2, OUTER_SIDESET_ID + side_index);
              else
                boundary_info.add_side(elem_Quad4, 2, OUTER_SIDESET_ID_ALT + side_index);
            }
          }
          else if (create_outward_interface_boundaries)
            boundary_info.add_side(elem_Quad4, 2, k * 2 + 1 + boundary_id_shift);
        }
      }
      j++;
    }
  }
}

Real
PolygonMeshGeneratorBase::radiusCorrectionFactor(const std::vector<Real> & azimuthal_list) const
{
  std::vector<Real> azimuthal_list_alt;
  Real tmp_acc = 0.0;
  azimuthal_list_alt.insert(azimuthal_list_alt.end(), azimuthal_list.begin(), azimuthal_list.end());
  azimuthal_list_alt.push_back(azimuthal_list.front() + 360.0);
  // summation of triangles S = 0.5 * r * r * Sigma_i [sin (azi_i)]
  // Circle area S_c = pi * r_0 * r_0
  // r = sqrt{2 * pi / Sigma_i [sin (azi_i)]} * r_0
  for (unsigned int i = 1; i < azimuthal_list_alt.size(); i++)
    tmp_acc += std::sin((azimuthal_list_alt[i] - azimuthal_list_alt[i - 1]) / 180.0 * M_PI);
  return std::sqrt(2 * M_PI / tmp_acc);
}

std::unique_ptr<ReplicatedMesh>
PolygonMeshGeneratorBase::buildSimplePeripheral(
    const unsigned int num_sectors_per_side,
    const unsigned int peripheral_invervals,
    const std::vector<std::pair<Real, Real>> & positions_inner,
    const std::vector<std::pair<Real, Real>> & d_positions_outer,
    const subdomain_id_type id_shift,
    const bool create_inward_interface_boundaries,
    const bool create_outward_interface_boundaries)
{
  auto mesh = buildReplicatedMesh(2);
  std::pair<Real, Real> positions_p;

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
      Elem * elem_Quad4 = mesh->add_elem(new Quad4);
      elem_Quad4->set_node(0) = mesh->node_ptr(j + (num_sectors_per_side + 1) * i);
      elem_Quad4->set_node(1) = mesh->node_ptr(j + 1 + (num_sectors_per_side + 1) * i);
      elem_Quad4->set_node(2) = mesh->node_ptr(j + 1 + (num_sectors_per_side + 1) * (i + 1));
      elem_Quad4->set_node(3) = mesh->node_ptr(j + (num_sectors_per_side + 1) * (i + 1));
      elem_Quad4->subdomain_id() = PERIPHERAL_ID_SHIFT + id_shift;
      if (i == 0)
      {
        boundary_info.add_side(elem_Quad4, 0, OUTER_SIDESET_ID);
        if (create_inward_interface_boundaries)
          boundary_info.add_side(elem_Quad4, 0, SLICE_ALT + id_shift * 2);
      }
      if (i == peripheral_invervals - 1)
      {
        boundary_info.add_side(elem_Quad4, 2, OUTER_SIDESET_ID);
        if (create_outward_interface_boundaries)
          boundary_info.add_side(elem_Quad4, 2, SLICE_ALT + id_shift * 2 + 1);
      }
      if (j == 0)
        boundary_info.add_side(elem_Quad4, 3, OUTER_SIDESET_ID);
      if (j == num_sectors_per_side - 1)
        boundary_info.add_side(elem_Quad4, 1, OUTER_SIDESET_ID);
    }
  }

  return mesh;
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
