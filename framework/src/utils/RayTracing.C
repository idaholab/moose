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

// Moose includes
#include "RayTracing.h"
#include "LineSegment.h"

// libMesh includes
#include "libmesh/plane.h"
#include "libmesh/point.h"
#include "libmesh/mesh.h"
#include "libmesh/point_locator_base.h"
#include "libmesh/elem.h"

namespace Moose
{

/**
 * Figure out which (if any) side of an Elem is intersected by a line.
 *
 * @param elem The elem to search
 * @param not_side A side to _not_ search (Use -1 if you want to search all sides)
 * @return The side that is intersected by the line.  Will return -1 if it doesn't intersect any side
 */
int sideIntersectedByLine(const Elem * elem, int not_side, const LineSegment & line_segment)
{
  unsigned int n_sides = elem->n_sides();

  // A Point to pass to the intersection method
  Point intersection_point;

  // Whether or not they intersect
  bool intersect = false;

  unsigned int dim = elem->dim();

  for (unsigned int i=0; i<n_sides; i++)
  {
    if (static_cast<int>(i) == not_side) // Don't search the "not_side"
      continue;

    // Get a simplified side element
    UniquePtr<Elem> side_elem = elem->side(i);

    if (dim == 3)
    {
      // Make a plane out of the first three nodes on the side
      Plane plane(side_elem->point(0), side_elem->point(1), side_elem->point(2));

      // See if they intersect
      intersect = line_segment.intersect(plane, intersection_point);
    }
    else if (dim == 2)
    {
      // Make a Line Segment out of the first two nodes on the side
      LineSegment side_segment(side_elem->point(0), side_elem->point(1));

      // See if they intersect
      intersect = line_segment.intersect(side_segment, intersection_point);
    }
    else // 1D
    {
      // See if the line segment contains the point
      intersect = line_segment.contains_point(side_elem->point(0));

      // If it does then save off that one point as the intersection point
      if (intersect)
        intersection_point = side_elem->point(0);
    }

    if (intersect)
      if (side_elem->contains_point(intersection_point))
        return i;
  }

  // Didn't find one
  return -1;
}

/**
 * Returns the side number for elem that neighbor is on
 *
 * Returns -1 if the neighbor can't be found to be a neighbor
 */
int sideNeighborIsOn(const Elem * elem, const Elem * neighbor)
{
  unsigned int n_sides = elem->n_sides();

  for (unsigned int i=0; i<n_sides; i++)
  {
    if (elem->neighbor(i) == neighbor)
      return i;
  }

  return -1;
}


/**
 * Recursively find all elements intersected by a line segment
 *
 * Works by moving from one element to the next _through_ the side of the current element.
 * This means that (other than for the first element) there is always an incoming_side
 * that is the reason we ended up in this element in the first place.  Search all the _other_
 * sides to see if there is a next element...
 *
 * @param line_segment the LineSegment to intersect
 * @param current_elem The current element that needs to be searched
 * @param incoming_side The side of the current element that was intersected by the LineSegment that brought us here
 * @param intersected_elems The output
 */
void recursivelyFindElementsIntersectedByLine(const LineSegment & line_segment, const Elem * current_elem, int incoming_side, std::vector<Elem *> & intersected_elems)
{
  // Find the side of this element that the LineSegment intersects... while ignoring the incoming side (we don't want to move backward!)
  int intersected_side = sideIntersectedByLine(current_elem, incoming_side, line_segment);

  if (intersected_side != -1) // -1 means that we didn't find any side
  {
    // Get the neighbor on that side
    Elem * neighbor = current_elem->neighbor(intersected_side);

    if (neighbor)
    {
      // Add it to the list
      intersected_elems.push_back(neighbor);

      // Note: This is finding the side the current_elem is on for the neighbor.  That's the "incoming_side" for the neighbor
      int incoming_side = sideNeighborIsOn(neighbor, current_elem);

      // Recurse
      recursivelyFindElementsIntersectedByLine(line_segment, neighbor, incoming_side, intersected_elems);

      return;
    }
  }

  // Finished... return out!
  return;
}

void elementsIntersectedByLine(const Point & p0, const Point & p1, const MeshBase & mesh, std::vector<Elem *> & intersected_elems)
{
  // Make sure our list is clear
  intersected_elems.clear();

  // Grab a PointLocator for finding the first element:
  const PointLocatorBase & pl = mesh.point_locator();

  // Find the starting element
  const Elem * first_elem = pl(p0);

  // Quick return if can't even locate the first element.
  if (!first_elem)
    return;

  intersected_elems.push_back(const_cast<Elem *>(first_elem));

  // Make a LineSegment object out of our two points for ease:
  LineSegment line_segment = LineSegment(p0, p1);

  // Find 'em!
  recursivelyFindElementsIntersectedByLine(line_segment, first_elem, -1, intersected_elems);
}

}
