/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "XFEMGeometricCut2D.h"

#include "MooseError.h"
#include "libmesh/string_to_enum.h"

XFEMGeometricCut2D::XFEMGeometricCut2D(Real x0, Real y0, Real x1, Real y1, Real t_start, Real t_end) :
    XFEMGeometricCut(t_start, t_end),
    _x0(x0),
    _x1(x1),
    _y0(y0),
    _y1(y1)
{
}

XFEMGeometricCut2D::~XFEMGeometricCut2D()
{
}

bool
XFEMGeometricCut2D::active(Real time)
{
  return cutFraction(time) > 0;
}

bool
XFEMGeometricCut2D::cutElementByGeometry(const Elem* elem, std::vector<CutEdge> & cut_edges, Real time)
{
  // Use the algorithm described here to determine whether edges are cut by the cut line:
  // http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect

  bool cut_elem = false;
  Real tol = 1.e-15;

  Real fraction = cutFraction(time);

  if (fraction > 0.0)
  {
    unsigned int n_sides = elem->n_sides();

    for (unsigned int i = 0; i < n_sides; ++i)
    {
      // This returns the lowest-order type of side, which should always
      // be an EDGE2 here because this class is for 2D only.
      UniquePtr<Elem> curr_side = elem->side(i);
      if (curr_side->type() != EDGE2)
        mooseError("In cutElementByGeometry element side must be EDGE2, but type is: " << libMesh::Utility::enum_to_string(curr_side->type())
                   << " base element type is: " << libMesh::Utility::enum_to_string(elem->type()));
      const Node *node1 = curr_side->get_node(0);
      const Node *node2 = curr_side->get_node(1);

      Real seg_originx = (*node1)(0);
      Real seg_originy = (*node1)(1);
      Real seg_endx = (*node2)(0);
      Real seg_endy = (*node2)(1);
      Real seg_dirx = seg_endx-seg_originx;
      Real seg_diry = seg_endy-seg_originy;

      Real cut_originx = _x0;
      Real cut_originy = _y0;
      Real cut_dirx = _x1 - _x0;
      Real cut_diry = _y1 - _y0;

      Real cut_origin_to_seg_originx = seg_originx-cut_originx;
      Real cut_origin_to_seg_originy = seg_originy-cut_originy;

      Real cut_dir_cross_seg_dir = crossProduct2D(cut_dirx, cut_diry, seg_dirx, seg_diry);

      if (std::abs(cut_dir_cross_seg_dir) > tol)
      {
        //Fraction of the distance along the cutting segment where it intersects the edge segment
        Real cut_int_frac = crossProduct2D(cut_origin_to_seg_originx, cut_origin_to_seg_originy, seg_dirx, seg_diry) /
          cut_dir_cross_seg_dir;

        if (cut_int_frac >= 0.0 && cut_int_frac <= fraction)
        { //Cutting segment intersects the line of the edge segment, but the intersection point may be outside the segment
          Real seg_int_frac = crossProduct2D(cut_origin_to_seg_originx, cut_origin_to_seg_originy, cut_dirx, cut_diry) /
            cut_dir_cross_seg_dir;
          if (seg_int_frac >= 0.0 && seg_int_frac <= 1.0) //TODO: revisit end cases for intersections with corners
          {
            cut_elem = true;
            CutEdge mycut;
            mycut.id1 = node1->id();
            mycut.id2 = node2->id();
            mycut.distance = seg_int_frac;
            mycut.host_side_id = i;
            cut_edges.push_back(mycut);
          }
        }
      }
    }
  }

  return cut_elem;
}

bool
XFEMGeometricCut2D::cutElementByGeometry(const Elem* /*elem*/, std::vector<CutFace> & /*cut_faces*/, Real /*time*/)
{
  mooseError("invalid method for 2D mesh cutting");
  return false;
}

bool
XFEMGeometricCut2D::cutFragmentByGeometry(std::vector<std::vector<Point> > & frag_edges,
                                          std::vector<CutEdge> & cut_edges, Real time)
{
  // Use the algorithm described here to determine whether edges are cut by the cut line:
  // http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect

  bool cut_frag = false;
  Real tol = 1.e-15;

  const Real fraction = cutFraction(time);

  if (fraction > 0.0)
  {
    unsigned int n_sides = frag_edges.size();

    for (unsigned int i = 0; i < n_sides; ++i)
    {
      Real seg_originx = frag_edges[i][0](0);
      Real seg_originy = frag_edges[i][0](1);
      Real seg_endx = frag_edges[i][1](0);
      Real seg_endy = frag_edges[i][1](1);
      Real seg_dirx = seg_endx-seg_originx;
      Real seg_diry = seg_endy-seg_originy;

      Real cut_originx = _x0;
      Real cut_originy = _y0;
      Real cut_dirx = _x1 - _x0;
      Real cut_diry = _y1 - _y0;

      Real cut_origin_to_seg_originx = seg_originx-cut_originx;
      Real cut_origin_to_seg_originy = seg_originy-cut_originy;

      Real cut_dir_cross_seg_dir = crossProduct2D(cut_dirx, cut_diry, seg_dirx, seg_diry);

      if (std::abs(cut_dir_cross_seg_dir) > tol)
      {
        //Fraction of the distance along the cutting segment where it intersects the edge segment
        Real cut_int_frac = crossProduct2D(cut_origin_to_seg_originx, cut_origin_to_seg_originy, seg_dirx, seg_diry) /
          cut_dir_cross_seg_dir;

        if (cut_int_frac >= 0.0 && cut_int_frac <= fraction)
        { //Cutting segment intersects the line of the edge segment, but the intersection point may be outside the segment
          Real seg_int_frac = crossProduct2D(cut_origin_to_seg_originx, cut_origin_to_seg_originy, cut_dirx, cut_diry) /
            cut_dir_cross_seg_dir;
          if (seg_int_frac >= 0.0 && seg_int_frac <= 1.0) //TODO: revisit end cases for intersections with corners
          {
            cut_frag = true;
            CutEdge mycut;
            mycut.id1 = i;
            mycut.id2 = (i < (n_sides - 1) ? (i + 1) : 0);
            mycut.distance = seg_int_frac;
            mycut.host_side_id = i;
            cut_edges.push_back(mycut);
          }
        }
      }
    }
  }

  return cut_frag;
}

bool
XFEMGeometricCut2D::cutFragmentByGeometry(std::vector<std::vector<Point> > & /*frag_faces*/,
                                            std::vector<CutFace> & /*cut_faces*/, Real /*time*/)
{
  mooseError("invalid method for 2D mesh cutting");
  return false;
}
