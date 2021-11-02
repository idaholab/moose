//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolygonMeshGeneratorBase.h"

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
PolygonMeshGeneratorBase::buildSimpleSlice(std::unique_ptr<ReplicatedMesh> mesh,
                                           const std::vector<Real> ring_radii,
                                           const std::vector<unsigned int> rings,
                                           std::vector<Real> ducts_center_dist,
                                           const std::vector<unsigned int> ducts_layers,
                                           const bool has_rings,
                                           const bool has_ducts,
                                           const Real pitch,
                                           const unsigned int num_sectors_per_side,
                                           const unsigned int background_intervals,
                                           dof_id_type * const node_id_background_meta,
                                           const unsigned int side_number,
                                           const unsigned int side_index,
                                           const std::vector<Real> azimuthal_tangent,
                                           const subdomain_id_type block_id_shift,
                                           const bool quad_center_elements,
                                           const boundary_id_type boundary_id_shift)
{
  unsigned int angle_number =
      azimuthal_tangent.size() == 0 ? num_sectors_per_side : (azimuthal_tangent.size() - 1);
  unsigned int ring_region_radial_intervals =
      has_rings ? accumulate(rings.begin(), rings.end(), 0) : 0;
  unsigned int duct_region_radial_intervals =
      has_ducts ? accumulate(ducts_layers.begin(), ducts_layers.end(), 0) : 0;

  unsigned int num_total_nodes = 0;

  // calculate the node number of the central elements.
  if (quad_center_elements)
  {
    num_total_nodes = (angle_number / 2 + 1) * (angle_number / 2 + 1) +
                      (angle_number + 1) * (ring_region_radial_intervals + background_intervals +
                                            duct_region_radial_intervals);
  }
  else
  {
    num_total_nodes =
        1 + (angle_number + 1) * (ring_region_radial_intervals + background_intervals +
                                  duct_region_radial_intervals);
  }

  std::vector<Node *> nodes(num_total_nodes); // reserve nodes pointers

  // Geometries
  const Real corner_to_corner =
      pitch / std::cos(M_PI / side_number); // distance of bin center to cell corner
  const Real corner_p[2][2] = {{0.0, 0.5 * corner_to_corner},
                               {0.5 * corner_to_corner * std::sin(2.0 * M_PI / side_number),
                                0.5 * corner_to_corner * std::cos(2.0 * M_PI / side_number)}};
  const unsigned int div_num = angle_number / 2 + 1;
  std::vector<std::vector<dof_id_type>> id_array(div_num, std::vector<dof_id_type>(div_num));
  dof_id_type node_id = 0;
  if (quad_center_elements)
  {
    Real ring_radii_0;

    if (has_rings)
      ring_radii_0 = ring_radii.front() / (Real)rings.front();
    else if (has_ducts)
      ring_radii_0 =
          ducts_center_dist.front() * std::cos(M_PI / side_number) / (Real)background_intervals;
    else
      ring_radii_0 = pitch / 2.0 / (Real)background_intervals;
    ring_radii_0 = ring_radii_0 / div_num * (div_num - 1);

    mesh = centerNodes(dynamic_pointer_cast<ReplicatedMesh>(mesh),
                       side_number,
                       div_num,
                       ring_radii_0,
                       &node_id,
                       &nodes,
                       &id_array);
  }
  else
  {
    Point center_p = Point(0.0, 0.0, 0.0); // pin-cell center
    nodes[0] = mesh->add_point(center_p, 0);
  }

  // create nodes for the ring regions
  if (has_rings)
    mesh = ringNodes(dynamic_pointer_cast<ReplicatedMesh>(mesh),
                     ring_radii,
                     rings,
                     num_sectors_per_side,
                     &node_id,
                     corner_p,
                     corner_to_corner,
                     &nodes,
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

  *node_id_background_meta = node_id;

  // create nodes for background region
  mesh = backgroundNodes(dynamic_pointer_cast<ReplicatedMesh>(mesh),
                         num_sectors_per_side,
                         background_intervals,
                         background_corner_distance,
                         background_corner_radial_interval_length,
                         &node_id,
                         corner_p,
                         corner_to_corner,
                         &nodes,
                         background_in,
                         azimuthal_tangent);

  // create nodes for duct regions
  if (has_ducts)
    mesh = ductNodes(dynamic_pointer_cast<ReplicatedMesh>(mesh),
                     &ducts_center_dist,
                     ducts_layers,
                     num_sectors_per_side,
                     &node_id,
                     corner_p,
                     corner_to_corner,
                     &nodes,
                     azimuthal_tangent);

  // Assign elements, boundaries, and subdomains;
  // Add Tri3 or Quad4 mesh into innermost (central) region
  if (quad_center_elements)
  {
    mesh = cenQuadElemDef(dynamic_pointer_cast<ReplicatedMesh>(mesh),
                          div_num,
                          nodes,
                          block_id_shift,
                          boundary_id_shift,
                          &id_array);
  }
  else
  {
    mesh = cenTriElemDef(dynamic_pointer_cast<ReplicatedMesh>(mesh),
                         num_sectors_per_side,
                         nodes,
                         azimuthal_tangent,
                         block_id_shift,
                         boundary_id_shift);
  }

  // Add Quad4 mesh into outer circle
  // total number of mesh should be all the rings for pin regions + background regions;
  // total number of quad mesh should be total number of mesh -1 (-1 is because the inner circle for
  // tri/quad mesh has been added above)

  std::vector<unsigned int> subdomain_rings;

  if (has_rings) //  define the rings in each subdomain
  {
    subdomain_rings = rings;
    subdomain_rings[0] = subdomain_rings[0] - 1;     // remove the inner TRI mesh subdomain
    subdomain_rings.push_back(background_intervals); // add the background region
  }
  else
  {
    subdomain_rings.push_back(background_intervals); // add the background region
    subdomain_rings[0] = subdomain_rings[0] - 1;     // remove the inner TRI mesh subdomain
  }

  if (has_ducts)
  {
    for (unsigned int i = 0; i < ducts_layers.size(); i++)
      subdomain_rings.push_back(ducts_layers[i]);
  }

  mesh = quadElemDef(dynamic_pointer_cast<ReplicatedMesh>(mesh),
                     num_sectors_per_side,
                     subdomain_rings,
                     nodes,
                     side_index,
                     azimuthal_tangent,
                     block_id_shift,
                     quad_center_elements ? (div_num * div_num - 1) : 0,
                     boundary_id_shift);

  return mesh;
}

std::unique_ptr<ReplicatedMesh>
PolygonMeshGeneratorBase::centerNodes(std::unique_ptr<ReplicatedMesh> mesh,
                                      const unsigned int side_number,
                                      const unsigned int div_num,
                                      const Real ring_radii_0,
                                      dof_id_type * const node_id,
                                      std::vector<Node *> * const nodes,
                                      std::vector<std::vector<dof_id_type>> * const id_array)
{
  const std::pair<Real, Real> p_origin = std::make_pair(0.0, 0.0);
  const std::pair<Real, Real> p_bottom =
      std::make_pair(0.0, ring_radii_0 * std::cos(M_PI / side_number));
  const std::pair<Real, Real> p_top =
      std::make_pair(p_bottom.second * std::sin(2.0 * M_PI / side_number),
                     p_bottom.second * std::cos(2.0 * M_PI / side_number));
  const std::pair<Real, Real> p_diag = std::make_pair(ring_radii_0 * std::sin(M_PI / side_number),
                                                      ring_radii_0 * std::cos(M_PI / side_number));

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
      Point p = Point(pc.first, pc.second, 0.0);
      (*nodes)[*node_id] = mesh->add_point(p, *node_id);
      (*id_array)[id_x][id_y] = *node_id;
      (*node_id)++;
      if (j < i)
        id_x++;
      if (j >= i)
        id_y--;
    }
  }
  (*node_id)--;
  return mesh;
}

std::unique_ptr<ReplicatedMesh>
PolygonMeshGeneratorBase::ringNodes(std::unique_ptr<ReplicatedMesh> mesh,
                                    const std::vector<Real> ring_radii,
                                    const std::vector<unsigned int> rings,
                                    const unsigned int num_sectors_per_side,
                                    dof_id_type * const node_id,
                                    const Real corner_p[2][2],
                                    const Real corner_to_corner,
                                    std::vector<Node *> * const nodes,
                                    const std::vector<Real> azimuthal_tangent)
{
  unsigned int angle_number =
      azimuthal_tangent.size() == 0 ? num_sectors_per_side : (azimuthal_tangent.size() - 1);
  // Add nodes in pins regions
  std::vector<Real> pin_radius_interval_length(rings.size());
  Real bin_radial_distance;
  for (unsigned int l = 0; l < rings.size(); l++)
  {
    if (l == 0)
      pin_radius_interval_length[l] =
          ring_radii[l] / rings[l]; // the pin radius interval for each ring_radii/subdomain
    else
      pin_radius_interval_length[l] =
          (ring_radii[l] - ring_radii[l - 1]) /
          rings[l]; // the pin radius interval for each ring_radii/subdomain

    // add rings in each pin subdomain
    for (unsigned int k = 0; k < rings[l]; k++)
    {
      (*node_id)++;
      if (l == 0)
        bin_radial_distance =
            ((k + 1) * pin_radius_interval_length[l]); // this is from the cell/pin center to
                                                       // the first circle
      else
        bin_radial_distance = (ring_radii[l - 1] + (k + 1) * pin_radius_interval_length[l]);
      const Real pin_corner_p_x = corner_p[0][0] * bin_radial_distance / (0.5 * corner_to_corner);
      const Real pin_corner_p_y = corner_p[0][1] * bin_radial_distance / (0.5 * corner_to_corner);

      // pin_corner_p(s) are the points in the pin region, on the bins towards the six corners,
      // at different intervals
      Point p = Point(pin_corner_p_x, pin_corner_p_y, 0.0);
      (*nodes)[*node_id] = mesh->add_point(p, *node_id);

      for (unsigned int j = 1; j <= angle_number; j++)
      {
        (*node_id)++;
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
        Point p = Point(pin_azimuthal_p_x, pin_azimuthal_p_y, 0.0);
        (*nodes)[*node_id] = mesh->add_point(p, *node_id);
      }
    }
  }
  return mesh;
}

std::unique_ptr<ReplicatedMesh>
PolygonMeshGeneratorBase::backgroundNodes(std::unique_ptr<ReplicatedMesh> mesh,
                                          const unsigned int num_sectors_per_side,
                                          const unsigned int background_intervals,
                                          const Real background_corner_distance,
                                          const Real background_corner_radial_interval_length,
                                          dof_id_type * const node_id,
                                          const Real corner_p[2][2],
                                          const Real corner_to_corner,
                                          std::vector<Node *> * const nodes,
                                          const Real background_in,
                                          const std::vector<Real> azimuthal_tangent)
{
  unsigned int angle_number =
      azimuthal_tangent.size() == 0 ? num_sectors_per_side : (azimuthal_tangent.size() - 1);
  for (unsigned int k = 0; k < (background_intervals); k++)
  {
    (*node_id)++;
    const Real background_corner_p_x =
        background_corner_distance / (0.5 * corner_to_corner) * corner_p[0][0] *
        (background_in + (k + 1) * background_corner_radial_interval_length) /
        background_corner_distance;
    const Real background_corner_p_y =
        background_corner_distance / (0.5 * corner_to_corner) * corner_p[0][1] *
        (background_in + (k + 1) * background_corner_radial_interval_length) /
        background_corner_distance;

    // background_corner_p(s) are the points in the background region, on the bins towards the six
    // corners, at different intervals
    Point p = Point(background_corner_p_x, background_corner_p_y, 0.0);
    (*nodes)[*node_id] = mesh->add_point(p, *node_id);

    for (unsigned int j = 1; j <= angle_number; j++)
    {
      (*node_id)++;
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
          cell_boundary_p_x * (background_in + (k + 1) * background_radial_interval) /
          std::sqrt(Utility::pow<2>(cell_boundary_p_x) + Utility::pow<2>(cell_boundary_p_y));
      const Real background_azimuthal_p_y =
          cell_boundary_p_y * (background_in + (k + 1) * background_radial_interval) /
          std::sqrt(Utility::pow<2>(cell_boundary_p_x) + Utility::pow<2>(cell_boundary_p_y));
      // background_azimuthal_p are the points on the bins towards different azimuthal angles, at
      // different intervals; excluding the ones produced by background_corner_p
      Point p = Point(background_azimuthal_p_x, background_azimuthal_p_y, 0.0);
      (*nodes)[*node_id] = mesh->add_point(p, *node_id);
    }
  }
  return mesh;
}

std::unique_ptr<ReplicatedMesh>
PolygonMeshGeneratorBase::ductNodes(std::unique_ptr<ReplicatedMesh> mesh,
                                    std::vector<Real> * const ducts_center_dist,
                                    const std::vector<unsigned int> ducts_layers,
                                    const unsigned int num_sectors_per_side,
                                    dof_id_type * const node_id,
                                    const Real corner_p[2][2],
                                    const Real corner_to_corner,
                                    std::vector<Node *> * const nodes,
                                    const std::vector<Real> azimuthal_tangent)
{
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
        (*node_id)++;

        bin_radial_distance = ((*ducts_center_dist)[l] + (k + 1) * duct_radius_interval_length[l]);
        const Real pin_corner_p_x = corner_p[0][0] * bin_radial_distance / (0.5 * corner_to_corner);
        const Real pin_corner_p_y = corner_p[0][1] * bin_radial_distance / (0.5 * corner_to_corner);

        // pin_corner_p(s) are the points in the pin region, on the bins towards the six corners,
        // at different intervals
        Point p = Point(pin_corner_p_x, pin_corner_p_y, 0.0);
        (*nodes)[*node_id] = mesh->add_point(p, *node_id);

        for (unsigned int j = 1; j <= angle_number; j++)
        {
          (*node_id)++;
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
          Point p = Point(pin_azimuthal_p_x, pin_azimuthal_p_y, 0.0);
          (*nodes)[*node_id] = mesh->add_point(p, *node_id);
        }
      }
    }
  }
  return mesh;
}

std::unique_ptr<ReplicatedMesh>
PolygonMeshGeneratorBase::cenQuadElemDef(std::unique_ptr<ReplicatedMesh> mesh,
                                         const unsigned int div_num,
                                         const std::vector<Node *> nodes,
                                         const subdomain_id_type block_id_shift,
                                         const boundary_id_type boundary_id_shift,
                                         std::vector<std::vector<dof_id_type>> * const id_array)
{

  BoundaryInfo & boundary_info = mesh->get_boundary_info();

  // This loop defines quad elements for the central regions except for the outermost layer
  for (unsigned int i = 0; i < div_num - 1; i++)
  {
    unsigned int id_x = 0;
    unsigned int id_y = i;
    for (unsigned int j = 0; j < 2 * i + 1; j++)
    {
      Elem * elem_Quad4 = mesh->add_elem(new Quad4);
      elem_Quad4->set_node(0) = nodes[(*id_array)[id_x][id_y]];
      elem_Quad4->set_node(3) = nodes[(*id_array)[id_x][id_y + 1]];
      elem_Quad4->set_node(2) = nodes[(*id_array)[id_x + 1][id_y + 1]];
      elem_Quad4->set_node(1) = nodes[(*id_array)[id_x + 1][id_y]];
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
    Elem * elem_Quad4 = mesh->add_elem(new Quad4);
    elem_Quad4->set_node(0) = nodes[i];
    elem_Quad4->set_node(3) = nodes[i + 2 * div_num - 1];
    elem_Quad4->set_node(2) = nodes[i + 2 * div_num];
    elem_Quad4->set_node(1) = nodes[i + 1];
    elem_Quad4->subdomain_id() = 1 + block_id_shift;
    boundary_info.add_side(elem_Quad4, 2, 1 + boundary_id_shift);
    if (i == (div_num - 1) * (div_num - 1))
      boundary_info.add_side(elem_Quad4, 3, SLICE_BEGIN);
    if (i == div_num * div_num - 2)
      boundary_info.add_side(elem_Quad4, 1, SLICE_END);
  }
  return mesh;
}

std::unique_ptr<ReplicatedMesh>
PolygonMeshGeneratorBase::cenTriElemDef(std::unique_ptr<ReplicatedMesh> mesh,
                                        const unsigned int num_sectors_per_side,
                                        const std::vector<Node *> nodes,
                                        const std::vector<Real> azimuthal_tangent,
                                        const subdomain_id_type block_id_shift,
                                        const boundary_id_type boundary_id_shift)
{
  unsigned int angle_number =
      azimuthal_tangent.size() == 0 ? num_sectors_per_side : (azimuthal_tangent.size() - 1);

  BoundaryInfo & boundary_info = mesh->get_boundary_info();
  for (unsigned int i = 1; i <= angle_number; i++)
  {
    Elem * elem = mesh->add_elem(new Tri3);
    elem->set_node(0) = nodes[0];
    elem->set_node(2) = nodes[i];
    elem->set_node(1) = nodes[i + 1];
    boundary_info.add_side(elem, 1, 1 + boundary_id_shift);
    elem->subdomain_id() = 1 + block_id_shift;
    if (i == 1)
      boundary_info.add_side(elem, 2, SLICE_BEGIN);
    else if (i == angle_number)
      boundary_info.add_side(elem, 0, SLICE_END);
  }
  return mesh;
}

std::unique_ptr<ReplicatedMesh>
PolygonMeshGeneratorBase::quadElemDef(std::unique_ptr<ReplicatedMesh> mesh,
                                      const unsigned int num_sectors_per_side,
                                      const std::vector<unsigned int> subdomain_rings,
                                      const std::vector<Node *> nodes,
                                      const unsigned int side_index,
                                      const std::vector<Real> azimuthal_tangent,
                                      const subdomain_id_type block_id_shift,
                                      const dof_id_type nodeid_shift,
                                      const boundary_id_type boundary_id_shift)
{
  unsigned int angle_number =
      azimuthal_tangent.size() == 0 ? num_sectors_per_side : (azimuthal_tangent.size() - 1);

  BoundaryInfo & boundary_info = mesh->get_boundary_info();
  unsigned int j = 0;
  for (unsigned int k = 0; k < (subdomain_rings.size()); k++)
  {
    for (unsigned int m = 0; m < subdomain_rings[k]; m++)
    {
      for (unsigned int i = 1; i <= angle_number; i++)
      {

        Elem * elem_Quad4 = mesh->add_elem(new Quad4);
        elem_Quad4->set_node(0) = nodes[nodeid_shift + i + (angle_number + 1) * j];
        elem_Quad4->set_node(1) = nodes[nodeid_shift + i + 1 + (angle_number + 1) * j];
        elem_Quad4->set_node(2) = nodes[nodeid_shift + i + (angle_number + 1) * (j + 1) + 1];
        elem_Quad4->set_node(3) = nodes[nodeid_shift + i + (angle_number + 1) * (j + 1)];
        if (i == 1)
          boundary_info.add_side(elem_Quad4, 3, SLICE_BEGIN);
        else if (i == angle_number)
          boundary_info.add_side(elem_Quad4, 1, SLICE_END);

        if (subdomain_rings[0] == 0)
          elem_Quad4->subdomain_id() = k + 1 + block_id_shift;
        else
          elem_Quad4->subdomain_id() = k + 2 + block_id_shift;

        if (m == (subdomain_rings[k] - 1))
        {
          if (k != (subdomain_rings.size() - 1))
          {
            if (subdomain_rings[0] == 0)
              boundary_info.add_side(elem_Quad4, 2, k + 1 + boundary_id_shift);
            else
              boundary_info.add_side(elem_Quad4, 2, k + 2 + boundary_id_shift);
          }
          else
          {
            boundary_info.add_side(elem_Quad4, 2, OUTER_SIDESET_ID);
            if (i <= angle_number / 2)
              boundary_info.add_side(elem_Quad4, 2, OUTER_SIDESET_ID + side_index);
            else
              boundary_info.add_side(elem_Quad4, 2, OUTER_SIDESET_ID_ALT + side_index);
          }
        }
      }
      j++;
    }
  }
  return mesh;
}

Real
PolygonMeshGeneratorBase::radiusCorrectionFactor(const std::vector<Real> azimuthal_list)
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
    std::unique_ptr<ReplicatedMesh> mesh,
    const unsigned int num_sectors_per_side,
    const unsigned int peripheral_invervals,
    const std::vector<std::pair<Real, Real>> positions_inner,
    const std::vector<std::pair<Real, Real>> d_positions_outer,
    std::vector<Node *> * const nodes,
    const subdomain_id_type id_shift)
{
  dof_id_type node_id = 0;
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
      Point p = Point(positions_p.first, positions_p.second, 0.0);
      (*nodes)[node_id] = mesh->add_point(p, node_id);
      node_id++;
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
      Point p = Point(positions_p.first, positions_p.second, 0.0);
      (*nodes)[node_id] = mesh->add_point(p, node_id);
      node_id++;
    }
  }
  node_id--;

  // element definition
  BoundaryInfo & boundary_info = mesh->get_boundary_info();

  for (unsigned int i = 0; i < peripheral_invervals; i++)
  {
    for (unsigned int j = 0; j < num_sectors_per_side; j++)
    {
      Elem * elem_Quad4 = mesh->add_elem(new Quad4);
      elem_Quad4->set_node(0) = (*nodes)[j + (num_sectors_per_side + 1) * i];
      elem_Quad4->set_node(1) = (*nodes)[j + 1 + (num_sectors_per_side + 1) * i];
      elem_Quad4->set_node(2) = (*nodes)[j + 1 + (num_sectors_per_side + 1) * (i + 1)];
      elem_Quad4->set_node(3) = (*nodes)[j + (num_sectors_per_side + 1) * (i + 1)];
      elem_Quad4->subdomain_id() = PERIPHERAL_ID_SHIFT + id_shift;
      if (i == 0)
      {
        boundary_info.add_side(elem_Quad4, 0, OUTER_SIDESET_ID);
        boundary_info.add_side(elem_Quad4, 0, SLICE_ALT + id_shift);
      }
      if (i == peripheral_invervals - 1)
      {
        boundary_info.add_side(elem_Quad4, 2, OUTER_SIDESET_ID);
        boundary_info.add_side(elem_Quad4, 2, SLICE_ALT + id_shift + 1);
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
                                           const unsigned int peripheral_invervals)
{
  auto position_px_inner =
      (pi_1_x * (num_sectors_per_side / 2.0 - j) + pi_2_x * j) / (num_sectors_per_side / 2.0);
  auto position_py_inner =
      (pi_1_y * (num_sectors_per_side / 2.0 - j) + pi_2_y * j) / (num_sectors_per_side / 2.0);
  auto position_px_outer =
      (d_po_1_x * (num_sectors_per_side / 2.0 - j) + d_po_2_x * j) / (num_sectors_per_side / 2.0);
  auto position_py_outer =
      (d_po_1_y * (num_sectors_per_side / 2.0 - j) + d_po_2_y * j) / (num_sectors_per_side / 2.0);
  auto position_px = position_px_inner + position_px_outer * i / peripheral_invervals;
  auto position_py = position_py_inner + position_py_outer * i / peripheral_invervals;
  return std::make_pair(position_px, position_py);
}

std::unique_ptr<ReplicatedMesh>
PolygonMeshGeneratorBase::addPeripheralMesh(
    const unsigned int pattern,
    const Real pitch,
    const std::vector<Real> extra_dist,
    const std::unique_ptr<ReplicatedMesh> * meshes,
    const std::vector<unsigned int> num_sectors_per_side_array,
    const std::vector<unsigned int> peripheral_duct_intervals,
    const Real rotation_angle,
    const unsigned int mesh_type)
{
  std::unique_ptr<MeshBase> out_meshes_ptr = (*meshes)->clone();
  std::unique_ptr<ReplicatedMesh> out_meshes = dynamic_pointer_cast<ReplicatedMesh>(out_meshes_ptr);
  std::vector<std::pair<Real, Real>> positions_inner;
  std::vector<std::pair<Real, Real>> d_positions_outer;

  std::vector<std::vector<unsigned int>> inner_index;
  std::vector<std::vector<unsigned int>> outer_index;
  std::vector<std::pair<Real, Real>> sub_positions_inner;
  std::vector<std::pair<Real, Real>> sub_d_positions_outer;
  unsigned int num_total_nodes_p;

  if (mesh_type == CORNER_MESH)
  {
    // corner mesh has three sides that need peripheral meshes.
    // each element has three sub-elements, representing beginning, middle, and ending azimuthal
    // points
    inner_index = {{0, 1, 2}, {2, 3, 4}, {4, 5, 6}};
    outer_index = {{0, 1, 2}, {2, 3, 4}, {4, 5, 6}};
  }
  else
  {
    // side mesh has two sides that need peripheral meshes.
    inner_index = {{0, 1, 2}, {2, 7, 8}};
    outer_index = {{0, 1, 2}, {2, 7, 8}};
  }

  // extra_dist includes background and ducts.
  // Loop to calculate the positions of the boundaries.
  for (unsigned int i = 0; i < extra_dist.size(); i++)
  {
    positionSetup(positions_inner,
                  d_positions_outer,
                  i == 0 ? 0.0 : extra_dist[i - 1],
                  extra_dist[i],
                  pitch,
                  i);

    num_total_nodes_p = (num_sectors_per_side_array[pattern] + 1) *
                        (peripheral_duct_intervals[i] + 1); // use uniformed azi first
    std::vector<Node *> nodes_p(num_total_nodes_p);
    // Loop for all applicable sides that need peripherial mesh (3 for corner and 2 for edge)
    for (unsigned int peripheral_index = 0; peripheral_index < inner_index.size();
         peripheral_index++)
    {
      // Loop for beginning, middle and ending positions of a side
      for (unsigned int vector_index = 0; vector_index < 3; vector_index++)
      {
        sub_positions_inner.push_back(positions_inner[inner_index[peripheral_index][vector_index]]);
        sub_d_positions_outer.push_back(
            d_positions_outer[outer_index[peripheral_index][vector_index]]);
      }
      auto mesh_p = buildReplicatedMesh(2); // create an empty 2D mesh
      auto meshp0 = buildSimplePeripheral(dynamic_pointer_cast<ReplicatedMesh>(mesh_p),
                                          num_sectors_per_side_array[pattern],
                                          peripheral_duct_intervals[i],
                                          sub_positions_inner,
                                          sub_d_positions_outer,
                                          &nodes_p,
                                          i);
      ReplicatedMesh other_mesh(*meshp0);
      // rotate the peripheral mesh to the desired side of the hexagon.
      MeshTools::Modification::rotate(other_mesh, rotation_angle, 0, 0);
      out_meshes->stitch_meshes(other_mesh, OUTER_SIDESET_ID, OUTER_SIDESET_ID, TOLERANCE, true);
      other_mesh.clear();
      sub_positions_inner.resize(0);
      sub_d_positions_outer.resize(0);
    }
  }
  return out_meshes;
}

void
PolygonMeshGeneratorBase::positionSetup(std::vector<std::pair<Real, Real>> & positions_inner,
                                        std::vector<std::pair<Real, Real>> & d_positions_outer,
                                        const Real extra_dist_in,
                                        const Real extra_dist_out,
                                        const Real pitch,
                                        const unsigned int radial_index)
{
  positions_inner.resize(0);
  d_positions_outer.resize(0);

  // Nine sets of positions are generated here, as shown below.
  // CORNER MESH Peripheral {0 1 2}, {2 3 4} and {4 5 6}
  //           3       2   1   0
  //            \      :   :   :
  //             \     :   :   :
  //      4.       .       :   :
  //         ` .               :
  //      5.   |               |
  //         ` |               |
  //      6.   |               |
  //         ` |               |
  //               .       .
  //                   .
  //
  // EDGE MESH Peripheral {0 1 2} and {2 7 8}
  //           8   7   2   1   0
  //           :   :   :   :   :
  //           :   :   :   :   :
  //           :   :       :   :
  //           :               :
  //           |               |
  //           |               |
  //           |               |
  //           |               |
  //               .       .
  //                   .

  positions_inner.push_back(
      std::make_pair(-pitch / 2.0,
                     std::sqrt(3.0) * pitch / 6.0 +
                         (radial_index != 0 ? std::sqrt(3.0) * pitch / 6.0 + extra_dist_in : 0.0)));
  positions_inner.push_back(std::make_pair(
      -pitch / 4.0,
      std::sqrt(3.0) * pitch / 4.0 +
          (radial_index != 0 ? std::sqrt(3.0) * pitch / 12.0 + extra_dist_in : 0.0)));
  positions_inner.push_back(std::make_pair(
      0.0, std::sqrt(3.0) * pitch / 3.0 + (radial_index != 0 ? extra_dist_in : 0.0)));
  positions_inner.push_back(std::make_pair(
      pitch / 4.0 + (radial_index != 0 ? pitch / 12.0 + std::sqrt(3.0) * extra_dist_in / 3.0 : 0.0),
      std::sqrt(3.0) * pitch / 4.0 +
          (radial_index != 0 ? pitch * std::sqrt(3.0) / 12.0 + extra_dist_in : 0.0)));
  positions_inner.push_back(std::make_pair(
      pitch / 2.0 + (radial_index != 0 ? std::sqrt(3.0) * extra_dist_in / 2.0 : 0.0),
      std::sqrt(3.0) * pitch / 6.0 + (radial_index != 0 ? extra_dist_in / 2.0 : 0.0)));
  positions_inner.push_back(std::make_pair(
      pitch / 2.0 + (radial_index != 0 ? pitch / 8.0 + std::sqrt(3.0) * extra_dist_in / 2.0 : 0.0),
      0.0 + (radial_index != 0 ? std::sqrt(3.0) * pitch / 24.0 + extra_dist_in / 2.0 : 0.0)));
  positions_inner.push_back(std::make_pair(
      pitch / 2.0 + (radial_index != 0 ? pitch / 4.0 + std::sqrt(3.0) * extra_dist_in / 2.0 : 0.0),
      -std::sqrt(3.0) * pitch / 6.0 +
          (radial_index != 0 ? std::sqrt(3.0) * pitch / 12.0 + extra_dist_in / 2.0 : 0.0)));
  positions_inner.push_back(std::make_pair(
      pitch / 4.0,
      std::sqrt(3.0) * pitch / 4.0 +
          (radial_index != 0 ? std::sqrt(3.0) * pitch / 12.0 + extra_dist_in : 0.0)));
  positions_inner.push_back(
      std::make_pair(pitch / 2.0,
                     std::sqrt(3.0) * pitch / 6.0 +
                         (radial_index != 0 ? std::sqrt(3.0) * pitch / 6.0 + extra_dist_in : 0.0)));

  d_positions_outer.push_back(
      std::make_pair(0.0,
                     std::sqrt(3.0) * pitch / 6.0 + extra_dist_out -
                         (radial_index != 0 ? std::sqrt(3.0) * pitch / 6.0 + extra_dist_in : 0.0)));
  d_positions_outer.push_back(std::make_pair(
      0.0,
      std::sqrt(3.0) * pitch / 12.0 + extra_dist_out -
          (radial_index != 0 ? std::sqrt(3.0) * pitch / 12.0 + extra_dist_in : 0.0)));
  d_positions_outer.push_back(
      std::make_pair(0.0, extra_dist_out - (radial_index != 0 ? extra_dist_in : 0.0)));
  d_positions_outer.push_back(std::make_pair(
      pitch / 12.0 + std::sqrt(3.0) * extra_dist_out / 3.0 -
          (radial_index != 0 ? pitch / 12.0 + std::sqrt(3.0) * extra_dist_in / 3.0 : 0.0),
      pitch * std::sqrt(3.0) / 12.0 + extra_dist_out -
          (radial_index != 0 ? pitch * std::sqrt(3.0) / 12.0 + extra_dist_in : 0.0)));
  d_positions_outer.push_back(
      std::make_pair(std::sqrt(3.0) * extra_dist_out / 2.0 -
                         (radial_index != 0 ? std::sqrt(3.0) * extra_dist_in / 2.0 : 0.0),
                     extra_dist_out / 2.0 - (radial_index != 0 ? extra_dist_in / 2.0 : 0.0)));
  d_positions_outer.push_back(std::make_pair(
      pitch / 8.0 + std::sqrt(3.0) * extra_dist_out / 2.0 -
          (radial_index != 0 ? pitch / 8.0 + std::sqrt(3.0) * extra_dist_in / 2.0 : 0.0),
      std::sqrt(3.0) * pitch / 24.0 + extra_dist_out / 2.0 -
          (radial_index != 0 ? std::sqrt(3.0) * pitch / 24.0 + extra_dist_in / 2.0 : 0.0)));
  d_positions_outer.push_back(std::make_pair(
      pitch / 4.0 + std::sqrt(3.0) * extra_dist_out / 2.0 -
          (radial_index != 0 ? pitch / 4.0 + std::sqrt(3.0) * extra_dist_in / 2.0 : 0.0),
      std::sqrt(3.0) * pitch / 12.0 + extra_dist_out / 2.0 -
          (radial_index != 0 ? std::sqrt(3.0) * pitch / 12.0 + extra_dist_in / 2.0 : 0.0)));
  d_positions_outer.push_back(std::make_pair(
      0.0,
      std::sqrt(3.0) * pitch / 12.0 + extra_dist_out -
          (radial_index != 0 ? std::sqrt(3.0) * pitch / 12.0 + extra_dist_in : 0.0)));
  d_positions_outer.push_back(
      std::make_pair(0.0,
                     std::sqrt(3.0) * pitch / 6.0 + extra_dist_out -
                         (radial_index != 0 ? std::sqrt(3.0) * pitch / 6.0 + extra_dist_in : 0.0)));
}

void
PolygonMeshGeneratorBase::nodeCoordRotate(Real * const x, Real * const y, const Real theta)
{
  const Real x_tmp = *x;
  const Real y_tmp = *y;
  *x = x_tmp * std::cos(theta * M_PI / 180.0) - y_tmp * std::sin(theta * M_PI / 180.0);
  *y = x_tmp * std::sin(theta * M_PI / 180.0) + y_tmp * std::cos(theta * M_PI / 180.0);
}

void
PolygonMeshGeneratorBase::cutOffHexDeform(MeshBase & mesh,
                                          const Real orientation,
                                          const Real y_max_0,
                                          const Real y_max_n,
                                          const Real y_min,
                                          const unsigned int mesh_type,
                                          const Real tols)
{
  for (libMesh::MeshBase::node_iterator it = mesh.nodes_begin(); it != mesh.nodes_end(); it++)
  {
    // This function can definitely be optimized in future for better efficiency.
    Real x_tmp = (**it)(0);
    Real y_tmp = (**it)(1);
    if (mesh_type == CORNER_MESH)
    {
      nodeCoordRotate(&x_tmp, &y_tmp, orientation);
      if (x_tmp >= 0.0 && y_tmp > y_max_0)
        y_tmp = y_tmp - y_max_0 + y_max_n;
      else if (x_tmp >= 0.0 && y_tmp >= y_min)
        y_tmp = (y_tmp - y_min) / (y_max_0 - y_min) * (y_max_n - y_min) + y_min;
      else if (y_tmp > -x_tmp * std::sqrt(3) + tols && y_tmp > y_max_0)
      {
        x_tmp /= y_tmp;
        y_tmp = y_tmp - y_max_0 + y_max_n;
        x_tmp *= y_tmp;
      }
      else if (y_tmp > -x_tmp * std::sqrt(3) + tols && y_tmp >= y_min)
      {
        x_tmp /= y_tmp;
        y_tmp = (y_tmp - y_min) / (y_max_0 - y_min) * (y_max_n - y_min) + y_min;
        x_tmp *= y_tmp;
      }
      nodeCoordRotate(&x_tmp, &y_tmp, -orientation);
      (**it)(0) = x_tmp;
      (**it)(1) = y_tmp;

      nodeCoordRotate(&x_tmp, &y_tmp, orientation - 60.0);
      if (x_tmp <= 0 && y_tmp > y_max_0)
        y_tmp = y_tmp - y_max_0 + y_max_n;
      else if (x_tmp <= 0 && y_tmp >= y_min)
        y_tmp = (y_tmp - y_min) / (y_max_0 - y_min) * (y_max_n - y_min) + y_min;
      else if (y_tmp >= x_tmp * std::sqrt(3) - tols && y_tmp > y_max_0)
      {
        x_tmp /= y_tmp;
        y_tmp = y_tmp - y_max_0 + y_max_n;
        x_tmp *= y_tmp;
      }
      else if (y_tmp >= x_tmp * std::sqrt(3) - tols && y_tmp >= y_min)
      {
        x_tmp /= y_tmp;
        y_tmp = (y_tmp - y_min) / (y_max_0 - y_min) * (y_max_n - y_min) + y_min;
        x_tmp *= y_tmp;
      }
      nodeCoordRotate(&x_tmp, &y_tmp, 60.0 - orientation);
      (**it)(0) = x_tmp;
      (**it)(1) = y_tmp;
    }
    else
    {
      nodeCoordRotate(&x_tmp, &y_tmp, orientation);
      if (y_tmp > y_max_0)
        y_tmp = y_tmp - y_max_0 + y_max_n;
      else if (y_tmp >= y_min)
        y_tmp = (y_tmp - y_min) / (y_max_0 - y_min) * (y_max_n - y_min) + y_min;
      nodeCoordRotate(&x_tmp, &y_tmp, -orientation);
      (**it)(0) = x_tmp;
      (**it)(1) = y_tmp;
    }
  }
}

std::pair<Real, Real>
PolygonMeshGeneratorBase::fourPointIntercept(const std::pair<Real, Real> p1,
                                             const std::pair<Real, Real> p2,
                                             const std::pair<Real, Real> p3,
                                             const std::pair<Real, Real> p4)
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
                                                   const Real lower_azi,
                                                   const Real upper_azi,
                                                   const unsigned int return_type,
                                                   const bool calculate_origin,
                                                   const Real input_origin_x,
                                                   const Real input_origin_y,
                                                   const Real tol) const
{
  std::vector<Real> azimuthal_output;
  std::vector<std::tuple<dof_id_type, unsigned short int, boundary_id_type>> side_list =
      mesh.get_boundary_info().build_side_list();
  mesh.get_boundary_info().build_node_list_from_side_list();
  std::vector<std::tuple<dof_id_type, boundary_id_type>> node_list =
      mesh.get_boundary_info().build_node_list();

  std::vector<Real> bd_x_list;
  std::vector<Real> bd_y_list;
  Real origin_x = 0.0;
  Real origin_y = 0.0;
  Real vol_tmp = 0.0;
  Real tmp_azi;
  const Real mid_azi = lower_azi <= upper_azi ? (lower_azi + upper_azi) / 2.0
                                              : (lower_azi + upper_azi + 360.0) / 2.0;
  for (unsigned int i = 0; i < node_list.size(); ++i)
    if (std::get<1>(node_list[i]) == OUTER_SIDESET_ID)
    {
      bd_x_list.push_back((mesh.node_ref(std::get<0>(node_list[i])))(0));
      bd_y_list.push_back((mesh.node_ref(std::get<0>(node_list[i])))(1));
    }

  if (calculate_origin)
  {
    // Iterate through elements to calculate the center of mass of the mesh, which is used as the
    // origin.
    for (const auto & elem : as_range(mesh.elements_begin(), mesh.elements_end()))
    {
      const auto volume = elem->volume();
      origin_x += elem->true_centroid()(0) * volume;
      origin_y += elem->true_centroid()(1) * volume;
      vol_tmp += volume;
    }
    origin_x /= vol_tmp;
    origin_y /= vol_tmp;
  }
  else
  {
    origin_x = input_origin_x;
    origin_y = input_origin_y;
  }

  for (unsigned int i = 0; i < bd_x_list.size(); ++i)
  {
    tmp_azi = atan2(bd_y_list[i] - origin_y, bd_x_list[i] - origin_x) * 180.0 / M_PI;
    if ((lower_azi <= upper_azi && (tmp_azi >= lower_azi - tol && tmp_azi <= upper_azi + tol)) ||
        (lower_azi > upper_azi && (tmp_azi >= lower_azi - tol || tmp_azi <= upper_azi + tol)))
    {
      azimuthal_output.push_back(
          return_type == ANGLE_DEGREE
              ? (tmp_azi - mid_azi)
              : (1.0 + std::sqrt(3.0) * std::tan((tmp_azi - mid_azi) / 180.0 * M_PI)));
    }
  }
  std::sort(azimuthal_output.begin(), azimuthal_output.end());

  return azimuthal_output;
}
