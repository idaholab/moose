//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Moose includes
#include "RayTracing.h"
#include "LineSegment.h"
#include "MooseError.h"

#include "libmesh/plane.h"
#include "libmesh/point.h"
#include "libmesh/mesh.h"
#include "libmesh/elem.h"

namespace Moose
{

/**
 * Figure out which (if any) side of an Elem is intersected by a line.
 *
 * @param elem The elem to search
 * @param not_side Sides to _not_ search (Use -1 if you want to search all sides)
 * @param intersection_point If an intersection is found this will be filled with the x,y,z position
 * of that intersection
 * @return The side that is intersected by the line.  Will return -1 if it doesn't intersect any
 * side
 */
int
sideIntersectedByLine(const Elem * elem,
                      std::vector<int> & not_side,
                      const LineSegment & line_segment,
                      Point & intersection_point)
{
  unsigned int n_sides = elem->n_sides();

  // Whether or not they intersect
  bool intersect = false;

  unsigned int dim = elem->dim();

  for (unsigned int i = 0; i < n_sides; i++)
  {
    // Don't search the "not_side"
    // Note: A linear search is fine here because this vector is going to be < n_sides
    if (std::find(not_side.begin(), not_side.end(), static_cast<int>(i)) != not_side.end())
      continue;

    // Get a simplified side element
    std::unique_ptr<const Elem> side_elem = elem->side_ptr(i);

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
    {
      if (side_elem->contains_point(intersection_point))
      {
        const Elem * neighbor = elem->neighbor_ptr(i);

        // If this side is on a boundary, let's do another search and see if we can find a better
        // candidate
        if (!neighbor)
        {
          not_side.push_back(i); // Make sure we don't find this side again

          int better_side = sideIntersectedByLine(elem, not_side, line_segment, intersection_point);

          if (better_side != -1)
            return better_side;
        }

        return i;
      }
    }
  }

  // Didn't find one
  return -1;
}

/**
 * Returns the side number for elem that neighbor is on
 *
 * Returns -1 if the neighbor can't be found to be a neighbor
 */
int
sideNeighborIsOn(const Elem * elem, const Elem * neighbor)
{
  unsigned int n_sides = elem->n_sides();

  for (unsigned int i = 0; i < n_sides; i++)
  {
    if (elem->neighbor_ptr(i) == neighbor)
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
 * @param incoming_side The side of the current element that was intersected by the LineSegment that
 * brought us here
 * @param intersected_elems The output
 * @param segments Line segments for the path across each element
 */
void
recursivelyFindElementsIntersectedByLine(const LineSegment & line_segment,
                                         const Elem * current_elem,
                                         int incoming_side,
                                         const Point & incoming_point,
                                         std::vector<Elem *> & intersected_elems,
                                         std::vector<LineSegment> & segments)
{
  Point intersection_point;

  std::vector<int> not_side(1, incoming_side);

  // Find the side of this element that the LineSegment intersects... while ignoring the incoming
  // side (we don't want to move backward!)
  int intersected_side =
      sideIntersectedByLine(current_elem, not_side, line_segment, intersection_point);

  if (intersected_side != -1) // -1 means that we didn't find any side
  {
    // Get the neighbor on that side
    const Elem * neighbor = current_elem->neighbor_ptr(intersected_side);

    if (neighbor)
    {
      // Add it to the list
      intersected_elems.push_back(const_cast<Elem *>(neighbor));

      // Add the line segment across the element to the segments list
      segments.push_back(LineSegment(incoming_point, intersection_point));

      // Note: This is finding the side the current_elem is on for the neighbor.  That's the
      // "incoming_side" for the neighbor
      int incoming_side = sideNeighborIsOn(neighbor, current_elem);

      // Recurse
      recursivelyFindElementsIntersectedByLine(
          line_segment, neighbor, incoming_side, intersection_point, intersected_elems, segments);
    }
    else // Add the final segment
      segments.push_back(LineSegment(incoming_point, line_segment.end()));
  }
  else // Add the final segment
    segments.push_back(LineSegment(incoming_point, line_segment.end()));

  // Finished... return out!
  return;
}

void
elementsIntersectedByLine(const Point & p0,
                          const Point & p1,
                          const MeshBase & /*mesh*/,
                          const PointLocatorBase & point_locator,
                          std::vector<Elem *> & intersected_elems,
                          std::vector<LineSegment> & segments)
{
  // Make sure our list is clear
  intersected_elems.clear();

  // Find the starting element
  const Elem * first_elem = point_locator(p0);

  // Quick return if can't even locate the first element.
  if (!first_elem)
    return;

  intersected_elems.push_back(const_cast<Elem *>(first_elem));

  // Make a LineSegment object out of our two points for ease:
  LineSegment line_segment = LineSegment(p0, p1);

  // Find 'em!
  recursivelyFindElementsIntersectedByLine(
      line_segment, first_elem, -1, p0, intersected_elems, segments);
}
}
