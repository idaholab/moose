#include "BetterDetailedSubChannelMesh.h"

#include <array>
#include <cmath>
#include "libmesh/cell_prism6.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("SubChannelApp", BetterDetailedSubChannelMesh);

InputParameters
BetterDetailedSubChannelMesh::validParams()
{
  InputParameters params = BetterQuadSubChannelMesh::validParams();
  return params;
}

BetterDetailedSubChannelMesh::BetterDetailedSubChannelMesh(const InputParameters & parameters)
  : BetterQuadSubChannelMesh(parameters)
{
}

void
BetterDetailedSubChannelMesh::buildMesh()
{
  UnstructuredMesh & mesh = dynamic_cast<UnstructuredMesh &>(getMesh());
  mesh.clear();
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  mesh.set_spatial_dimension(3);
  // Define the resolution (the number of points used to represent a circle).
  // This must be divisible by 4.
  const int theta_res = 8; // TODO: parameterize
  // Compute the number of points needed to represent one quarter of a circle.
  const int points_per_quad = theta_res / 4 + 1;
  // Compute the points needed to represent one axial slice of a subchannel.
  // There is one center point plus the points from 4 intersecting circles.
  const int points_per_plane = points_per_quad * 4 + 1;
  // Compute the total number of points and elements.
  const int points_per_ch = points_per_plane * (_n_cells + 1);
  const int n_points = points_per_ch * _n_channels;
  const int elems_per_level = theta_res + 4;
  const int n_elems = elems_per_level * _n_cells * _n_channels;
  mesh.reserve_nodes(n_points);
  mesh.reserve_elem(n_elems);
  // Build an array of points aranged in a circle on the xy-plane.
  double radius = 0.0045720; // TODO: generalize
  std::array<Point, theta_res + 1> circle_points;
  {
    double theta = 0;
    for (int i = 0; i < theta_res + 1; i++)
    {
      circle_points[i](0) = radius * std::cos(theta);
      circle_points[i](1) = radius * std::sin(theta);
      theta += 2 * M_PI / theta_res;
    }
  }
  // Define "quadrant center" reference points.  These will be the centers of
  // the 4 circles that intersect each a subchannel cell.  These centers are
  // offset a little bit so that in the final mesh, there is a tiny gap between
  // neighboring subchannel cells.  That allows us to easily map a solution to
  // this detailed mesh with a nearest-neighbor search.
  std::array<Point, 4> quadrant_centers;
  quadrant_centers[0] = Point(_pitch * 0.5 * 0.99999, _pitch * 0.5 * 0.99999, 0);
  quadrant_centers[1] = Point(-_pitch * 0.5 * 0.99999, _pitch * 0.5 * 0.99999, 0);
  quadrant_centers[2] = Point(-_pitch * 0.5 * 0.99999, -_pitch * 0.5 * 0.99999, 0);
  quadrant_centers[3] = Point(_pitch * 0.5 * 0.99999, -_pitch * 0.5 * 0.99999, 0);
  // Build an array of points that represent an axial slice of a subchannel
  // cell.  The points are ordered in this fashion:
  //     4   3
  // 6 5       2 1
  //       0
  // 7 8       * *
  //     9   *
  std::array<Point, points_per_plane> plane_points;
  {
    int n = points_per_quad;
    int m = theta_res / 4;
    for (int i = 0; i < n; i++)
    {
      auto c_pt = circle_points[3 * m - i];
      plane_points[i + 1] = quadrant_centers[0] + c_pt;
    }
    for (int i = 0; i < n; i++)
    {
      auto c_pt = circle_points[4 * m - i];
      plane_points[i + n + 1] = quadrant_centers[1] + c_pt;
    }
    for (int i = 0; i < n; i++)
    {
      auto c_pt = circle_points[m - i];
      plane_points[i + 2 * n + 1] = quadrant_centers[2] + c_pt;
    }
    for (int i = 0; i < n; i++)
    {
      auto c_pt = circle_points[2 * m - i];
      plane_points[i + 3 * n + 1] = quadrant_centers[3] + c_pt;
    }
  }
  // Add the points to the mesh.
  unsigned int node_id = 0;
  Real offset_x = (_nx - 1) * _pitch / 2.0;
  Real offset_y = (_ny - 1) * _pitch / 2.0;
  for (unsigned int iy = 0; iy < _ny; iy++)
  {
    Point y0 = {0, _pitch * iy - offset_y, 0};
    for (unsigned int ix = 0; ix < _nx; ix++)
    {
      Point x0 = {_pitch * ix - offset_x, 0, 0};
      for (auto z : _z_grid)
      {
        Point z0{0, 0, z};
        for (int i = 0; i < points_per_plane; i++)
        {
          mesh.add_point(plane_points[i] + x0 + y0 + z0, node_id++);
        }
      }
    }
  }
  // Add the elements to the mesh.  The elements are 6-node prisms.  The
  // bases of these prisms form a triangulated representation of an axial
  // slice of a subchannel.
  unsigned int elem_id = 0;
  for (unsigned int iy = 0; iy < _ny; iy++)
  {
    for (unsigned int ix = 0; ix < _nx; ix++)
    {
      int i_ch = _nx * iy + ix;
      for (unsigned int iz = 0; iz < _n_cells; iz++)
      {
        for (int i = 0; i < elems_per_level; i++)
        {
          Elem * elem = new Prism6;
          elem->set_id(elem_id++);
          elem = mesh.add_elem(elem);

          const int indx1 = iz * points_per_plane + points_per_ch * i_ch;
          const int indx2 = (iz + 1) * points_per_plane + points_per_ch * i_ch;

          elem->set_node(0) = mesh.node_ptr(indx1);
          elem->set_node(1) = mesh.node_ptr(indx1 + i + 1);
          if (i != elems_per_level - 1)
          {
            elem->set_node(2) = mesh.node_ptr(indx1 + i + 2);
          }
          else
          {
            elem->set_node(2) = mesh.node_ptr(indx1 + 1);
          }
          elem->set_node(3) = mesh.node_ptr(indx2);
          elem->set_node(4) = mesh.node_ptr(indx2 + i + 1);
          if (i != elems_per_level - 1)
          {
            elem->set_node(5) = mesh.node_ptr(indx2 + i + 2);
          }
          else
          {
            elem->set_node(5) = mesh.node_ptr(indx2 + 1);
          }

          if (iz == 0)
            boundary_info.add_side(elem, 0, 0);
          if (iz == _n_cells - 1)
            boundary_info.add_side(elem, 4, 1);
        }
      }
    }
  }
  boundary_info.sideset_name(0) = "inlet";
  boundary_info.sideset_name(1) = "outlet";
  mesh.prepare_for_use();
}
