//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/mesh_base.h"
#include "libmesh/boundary_info.h"
#include "libmesh/replicated_mesh.h"

#include "MooseUtils.h"
#include "MooseTypes.h"
#include "FaceInfo.h"

namespace MooseMeshXYCuttingUtils
{
/**
 * Removes all the elements on one side of a given line and deforms the elements intercepted by
 * the line to form a flat new boundary
 * @param mesh input mesh to perform line-based elements removing on
 * @param bdry_pars line parameter sets {a, b, c} as in a*x+b*y+c=0
 * @param block_id_to_remove subdomain id used to mark the elements that need to be removed
 * @param subdomain_ids_set all the subdomain ids in the input mesh
 * @param trimming_section_boundary_id ID of the new external boundary formed due to
 * trimming
 * @param external_boundary_id ID of the external boundary of the input mesh
 * @param other_boundaries_to_conform IDs of the other boundaries that need to be conformed to
 * during nodes moving
 * @param assign_ext_to_new whether to assign external_boundary_id to the new boundary formed by
 * removal
 * @param side_to_remove which side of the mesh needs to be removed: true means ax+by+c>0 and
 * false means ax+by+c<0
 */
void lineRemoverMoveNode(ReplicatedMesh & mesh,
                         const std::vector<Real> & bdry_pars,
                         const subdomain_id_type block_id_to_remove,
                         const std::set<subdomain_id_type> & subdomain_ids_set,
                         const boundary_id_type trimming_section_boundary_id,
                         const boundary_id_type external_boundary_id,
                         const std::vector<boundary_id_type> & other_boundaries_to_conform =
                             std::vector<boundary_id_type>(),
                         const bool assign_ext_to_new = false,
                         const bool side_to_remove = true);

/**
 * Determines whether a point on XY-plane is on the side of a given line that needs to be removed
 * @param px x coordinate of the point
 * @param py y coordinate of the point
 * @param param_1 parameter 1 (a) in line formula a*x+b*y+c=0
 * @param param_2 parameter 2 (b) in line formula a*x+b*y+c=0
 * @param param_3 parameter 3 (c) in line formula a*x+b*y+c=0
 * @param direction_param which side is the side that needs to be removed
 * @param dis_tol tolerance used in determining side
 * @return whether the point is on the side of the line that needed to be removed
 */
bool lineSideDeterminator(const Real px,
                          const Real py,
                          const Real param_1,
                          const Real param_2,
                          const Real param_3,
                          const bool direction_param,
                          const Real dis_tol = libMesh::TOLERANCE);

/**
 * Calculates the intersection Point of two given straight lines
 * @param param_11 parameter 1 (a) in line formula a*x+b*y+c=0 for the first line
 * @param param_12 parameter 2 (b) in line formula a*x+b*y+c=0 for the first line
 * @param param_13 parameter 3 (c) in line formula a*x+b*y+c=0 for the first line
 * @param param_21 parameter 1 (a) in line formula a*x+b*y+c=0 for the second line
 * @param param_22 parameter 2 (b) in line formula a*x+b*y+c=0 for the second line
 * @param param_23 parameter 3 (c) in line formula a*x+b*y+c=0 for the second line
 * @return intersect point of the two lines
 */
Point twoLineIntersection(const Real param_11,
                          const Real param_12,
                          const Real param_13,
                          const Real param_21,
                          const Real param_22,
                          const Real param_23);

/**
 * Calculates the intersection Point of a straight line defined by two given points and another
 * straight line
 * @param pt1 point 1 that defines the first straight line
 * @param pt2 point 2 that defines the first straight line
 * @param param_1 parameter 1 (a) in line formula a*x+b*y+c=0 for the second straight line
 * @param param_2 parameter 2 (b) in line formula a*x+b*y+c=0 for the second straight line
 * @param param_3 parameter 3 (c) in line formula a*x+b*y+c=0 for the second straight line
 * @return intersect point of the two lines
 */
Point twoPointandLineIntersection(const Point & pt1,
                                  const Point & pt2,
                                  const Real param_1,
                                  const Real param_2,
                                  const Real param_3);

/**
 * Fixes degenerate QUAD elements created by the hexagonal mesh trimming by converting them into
 * TRI elements
 * @param mesh input mesh with degenerate QUAD elements that need to be fixed
 * @param subdomain_ids_set all the subdomain ids in the input mesh
 * @param tri_elem_subdomain_shift subdomain id shift used to define the TRI element subdomains
 * @param tri_elem_subdomain_name_suffix suffix used to name the TRI element subdomains
 * @return whether any elements have been fixed
 */
bool
quasiTriElementsFixer(ReplicatedMesh & mesh,
                      const std::set<subdomain_id_type> & subdomain_ids_set,
                      const subdomain_id_type tri_elem_subdomain_shift = Moose::INVALID_BLOCK_ID,
                      const SubdomainName tri_elem_subdomain_name_suffix = "tri");

/**
 * Calculates the internal angles of a given 2D element
 * @param elem the element that needs to be investigated
 * @return sizes of all the internal angles, sorted by their size
 */
std::vector<std::pair<Real, unsigned int>> vertex_angles(const Elem & elem);

/**
 * Calculates the distances between the vertices of a given 2D element
 * @param elem the element that needs to be investigated
 * @return values of all the distances, sorted by their value
 */
std::vector<std::pair<Real, unsigned int>> vertex_distances(const Elem & elem);

/**
 * Split a TRI3 element into three TRI3 elements based on two nodes on the two sides of the triangle
 * @param mesh input mesh with the TRI3 element that needs to be split
 * @param elem_id id of the TRI3 element that needs to be split
 * @param node_shift shift used to rotate the vertices to make sure the vertex that the two cut
 * sides share is the first vertex
 * @param nid_3 id of the node on the first cut side of the triangle
 * @param nid_4 id of the node on the second cut side of the triangle
 * @param single_elem_side_id subdomain id of the single element side
 * @param double_elem_side_id subdomain id of the double element side
 * @param new_boundary_id boundary id of the new boundary that divides the single element side and
 * the double element side
 */
void triElemSplitter(ReplicatedMesh & mesh,
                     const dof_id_type elem_id,
                     const unsigned short node_shift,
                     const dof_id_type nid_3,
                     const dof_id_type nid_4,
                     const subdomain_id_type single_elem_side_id,
                     const subdomain_id_type double_elem_side_id,
                     const boundary_id_type new_boundary_id);

/**
 * Split a TRI3 element into two TRI3 elements based on one node on one side of the triangle
 * @param mesh input mesh with the TRI3 element that needs to be split
 * @param elem_id id of the TRI3 element that needs to be split
 * @param node_shift shift used to rotate the vertices to make sure the vertex corresponding to the
 * cut side is the first vertex
 * @param nid_m id of the node on the cut side of the triangle
 * @param first_elem_side_id subdomain id of the first element side
 * @param second_elem_side_id subdomain id of the second element side
 * @param new_boundary_id boundary id of the new boundary that divides the two elements
 */
void triElemSplitter(ReplicatedMesh & mesh,
                     const dof_id_type elem_id,
                     const unsigned short node_shift,
                     const dof_id_type nid_m,
                     const subdomain_id_type first_elem_side_id,
                     const subdomain_id_type second_elem_side_id,
                     const boundary_id_type new_boundary_id);

/**
 * Split a QUAD4 element into two TRI3 elements
 * @param mesh input mesh with the QUAD4 element that needs to be split
 * @param elem_id id of the QUAD4 element that needs to be split
 * @param tri_elem_subdomain_shift subdomain id shift used to define the TRI element subdomains
 */
void quadElemSplitter(ReplicatedMesh & mesh,
                      const dof_id_type elem_id,
                      const subdomain_id_type tri_elem_subdomain_shift);

/**
 * Convert all the QUAD4 elements in the mesh that are crossed by the given line into TRI3 elements
 * @param mesh input mesh with the QUAD4 elements that need to be converted
 * @param cut_line_params parameters of the line that cuts the input mesh
 * @param tri_subdomain_id_shift subdomain id shift used to define the TRI element subdomains
 * generated due to the conversion
 * @param tri_elem_subdomain_name_suffix suffix used to name the TRI element subdomains generated
 * due to the conversion
 */
void quadToTriOnLine(ReplicatedMesh & mesh,
                     const std::vector<Real> & cut_line_params,
                     const dof_id_type tri_subdomain_id_shift,
                     const SubdomainName tri_elem_subdomain_name_suffix);

/**
 * Trim the 2D mesh by removing all the elements on one side of the given line. Note that the mesh
 * needs to be pre-processed so that only TRI3 are crossed by the given line
 * @param mesh input mesh that need to be trimmed
 * @param cut_line_params parameters of the line that cuts the input mesh
 * @param block_id_to_remove a temporary subdomain id used to mark the elements that need to be
 * removed
 * @param new_boundary_id boundary id of the new boundary that forms due to the trimming
 */
void lineRemoverCutElemTri(ReplicatedMesh & mesh,
                           const std::vector<Real> & cut_line_params,
                           const subdomain_id_type block_id_to_remove,
                           const boundary_id_type new_boundary_id);

/**
 * Trim the 2D mesh by removing all the elements on one side of the given line. Note that the mesh
 * can only contain QUAD4 and TRI3 elements
 * @param mesh input mesh that need to be trimmed
 * @param cut_line_params parameters of the line that cuts the input mesh
 * @param tri_subdomain_id_shift subdomain id shift used to define the TRI element subdomains formed
 * due to the trimming
 * @param tri_elem_subdomain_name_suffix suffix used to name the TRI element subdomains formed due
 * to the trimming
 * @param block_id_to_remove a temporary subdomain id used to mark the elements that need to be
 * removed
 * @param new_boundary_id boundary id of the new boundary that forms due to the trimming
 * @param improve_boundary_tri_elems flag to indicate whether the boundary TRI3 elements need to be
 * improved
 */
void lineRemoverCutElem(ReplicatedMesh & mesh,
                        const std::vector<Real> & cut_line_params,
                        const dof_id_type tri_subdomain_id_shift,
                        const SubdomainName tri_elem_subdomain_name_suffix,
                        const subdomain_id_type block_id_to_remove,
                        const boundary_id_type new_boundary_id,
                        const bool improve_boundary_tri_elems = false);

/**
 * Improve the element quality of the boundary TRI3 elements of the given boundary
 * @param mesh input mesh with the boundary TRI3 elements that need to be improved
 * @param boundary_to_improve boundary id of the boundary that needs to be improved
 */
void boundaryTriElemImprover(ReplicatedMesh & mesh, const boundary_id_type boundary_to_improve);

/**
 * Make a TRI3 element with the given node ids and subdomain id with boundary information
 * @param mesh input mesh where the TRI3 element needs to be added
 * @param node_id_0 id of the first node of the TRI3 element
 * @param node_id_1 id of the second node of the TRI3 element
 * @param node_id_2 id of the third node of the TRI3 element
 * @param subdomain_id subdomain id of the TRI3 element
 * @param extra_elem_ids extra element ids to be assigned to the TRI3 element
 * @param boundary_ids_for_side_1 boundary ids of the second side of the TRI3 element
 * @param boundary_ids_for_side_0 boundary ids of the first side of the TRI3 element
 * @param boundary_ids_for_side_2 boundary ids of the third side of the TRI3 element
 */
void makeImprovedTriElement(
    ReplicatedMesh & mesh,
    const dof_id_type node_id_0,
    const dof_id_type node_id_1,
    const dof_id_type node_id_2,
    const subdomain_id_type subdomain_id,
    const std::vector<dof_id_type> & extra_elem_ids,
    const std::vector<boundary_id_type> & boundary_ids_for_side_1 = std::vector<boundary_id_type>(),
    const std::vector<boundary_id_type> & boundary_ids_for_side_0 = std::vector<boundary_id_type>(),
    const std::vector<boundary_id_type> & boundary_ids_for_side_2 =
        std::vector<boundary_id_type>());

/**
 * Check if there is a side in an element that contains the given pair of nodes; if yes, also find
 * the side id and the direction of the two nodes in the side
 * @param mesh input mesh with the element that needs to be checked
 * @param elem_id id of the element that needs to be checked
 * @param node_id_0 id of the first node of the pair
 * @param node_id_1 id of the second node of the pair
 * @param side_id id of the side that contains the pair of nodes
 * @param is_inverse flag to indicate if the two nodes are in the same direction as the side
 * @return true if the element contains the side with the given pair of nodes
 */
bool elemSideLocator(ReplicatedMesh & mesh,
                     const dof_id_type elem_id,
                     const dof_id_type node_id_0,
                     const dof_id_type node_id_1,
                     unsigned short & side_id,
                     bool & is_inverse);
}
