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

#include "libmesh/mesh_base.h"
#include "XFEM_geometric_cut.h"

XFEM_geometric_cut::XFEM_geometric_cut ()
{
  x0 =  0.2;
  y0 =  0.0;
  x1 = -0.3;
  y1 =  0.1;

  t_start = 0.0;
  t_end = 0.0;
}

XFEM_geometric_cut::XFEM_geometric_cut (Real x0_, Real y0_, Real x1_, Real y1_, Real t_start_, Real t_end_):
  x0(x0_),
  x1(x1_),
  y0(y0_),
  y1(y1_),
  t_start(t_start_),
  t_end(t_end_)
{}

XFEM_geometric_cut::~XFEM_geometric_cut ()
{}

Real XFEM_geometric_cut::crossprod_2d(Real ax, Real ay, Real bx, Real by)
{
  return (ax*by-bx*ay);
}

Real XFEM_geometric_cut::cut_fraction(Real time)
{
  Real fraction = 0.0;
  if (time > t_start)
  {
    if (time >= t_end)
    {
      fraction = 1.0;
    }
    else
    {
      fraction = (time - t_start) / (t_end - t_start);
    }
  }
  return fraction;
}

bool XFEM_geometric_cut::cut_elem_by_geometry(const Elem* elem, std::vector<cutEdge> & cutEdges, Real time)
{
  //Use the algorithm described here to determine whether edges are cut by the cut line:
  //http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect

  bool cut_elem = false;
  Real tol = 1.e-15;

  Real fraction = cut_fraction(time);

  unsigned int nsides = elem->n_nodes();

  for (unsigned int i = 0; i < nsides; ++i)
  {
    Node *node1 = elem->get_node(i);
    Node *node2 = elem->get_node(i<(nsides-1) ? i+1 : 0);

    Real seg_originx = (*node1)(0);
    Real seg_originy = (*node1)(1);
    Real seg_endx = (*node2)(0);
    Real seg_endy = (*node2)(1);
    Real seg_dirx = seg_endx-seg_originx;
    Real seg_diry = seg_endy-seg_originy;

    Real cut_originx = x0;
    Real cut_originy = y0;
    Real cut_dirx = x1-x0;
    Real cut_diry = y1-y0;

    Real cut_origin_to_seg_originx = seg_originx-cut_originx;
    Real cut_origin_to_seg_originy = seg_originy-cut_originy;

    Real cut_dir_cross_seg_dir = crossprod_2d(cut_dirx, cut_diry, seg_dirx, seg_diry);

    if (fabs(cut_dir_cross_seg_dir) > tol)
    {
      //Fraction of the distance along the cutting segment where it intersects the edge segment
      Real cut_int_frac = crossprod_2d(cut_origin_to_seg_originx, cut_origin_to_seg_originy, seg_dirx, seg_diry) /
                          cut_dir_cross_seg_dir;

      if (cut_int_frac >= 0.0 && cut_int_frac <= fraction)
      { //Cutting segment intersects the line of the edge segment, but the intersection point may be outside the segment
        Real seg_int_frac = crossprod_2d(cut_origin_to_seg_originx, cut_origin_to_seg_originy, cut_dirx, cut_diry) /
                            cut_dir_cross_seg_dir;
        if (seg_int_frac >= 0.0 && seg_int_frac <= 1.0) //TODO: revisit end cases for intersections with corners
        {

          cut_elem = true;
          cutEdge mycut;
          mycut.id1 = node1->id();
          mycut.id2 = node2->id();
          mycut.distance = seg_int_frac;
          mycut.host_side_id = i;
          cutEdges.push_back(mycut);
        }
      }
    }
  }

  return cut_elem;
}

bool XFEM_geometric_cut::cut_frag_by_geometry(std::vector<std::vector<Point> > & frag_edges, 
                                              std::vector<cutEdge> & cutEdges, Real time)
{
  //Use the algorithm described here to determine whether edges are cut by the cut line:
  //http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect

  bool cut_frag = false;
  Real tol = 1.e-15;

  Real fraction = cut_fraction(time);

  unsigned int nsides = frag_edges.size();

  for (unsigned int i=0; i<nsides; ++i)
  {
    Real seg_originx = frag_edges[i][0](0);
    Real seg_originy = frag_edges[i][0](1);
    Real seg_endx = frag_edges[i][1](0);
    Real seg_endy = frag_edges[i][1](1);
    Real seg_dirx = seg_endx-seg_originx;
    Real seg_diry = seg_endy-seg_originy;

    Real cut_originx = x0;
    Real cut_originy = y0;
    Real cut_dirx = x1-x0;
    Real cut_diry = y1-y0;

    Real cut_origin_to_seg_originx = seg_originx-cut_originx;
    Real cut_origin_to_seg_originy = seg_originy-cut_originy;

    Real cut_dir_cross_seg_dir = crossprod_2d(cut_dirx, cut_diry, seg_dirx, seg_diry);

    if (fabs(cut_dir_cross_seg_dir) > tol)
    {
      //Fraction of the distance along the cutting segment where it intersects the edge segment
      Real cut_int_frac = crossprod_2d(cut_origin_to_seg_originx, cut_origin_to_seg_originy, seg_dirx, seg_diry) /
                          cut_dir_cross_seg_dir;

      if (cut_int_frac >= 0.0 && cut_int_frac <= fraction)
      { //Cutting segment intersects the line of the edge segment, but the intersection point may be outside the segment
        Real seg_int_frac = crossprod_2d(cut_origin_to_seg_originx, cut_origin_to_seg_originy, cut_dirx, cut_diry) /
                            cut_dir_cross_seg_dir;
        if (seg_int_frac >= 0.0 && seg_int_frac <= 1.0) //TODO: revisit end cases for intersections with corners
        {

          cut_frag = true;
          cutEdge mycut;
          mycut.id1 = i;
          mycut.id2 = (i<(nsides-1) ? (i+1) : 0);
          mycut.distance = seg_int_frac;
          mycut.host_side_id = i;
          cutEdges.push_back(mycut);
        }
      }
    }
  }

  return cut_frag;
}
