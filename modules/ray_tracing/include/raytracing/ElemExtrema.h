//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RayTracingCommon.h"

// MOOSE includes
#include "MooseTypes.h"

namespace libMesh
{
class Elem;
}

/**
 * Helper for defining if at an element's edge, vertex, or neither
 */
struct ElemExtrema : std::pair<unsigned short, unsigned short>
{
  /**
   * Default constructor: sets entires to invalid (not at vertex or edge)
   */
  ElemExtrema()
    : std::pair<unsigned short, unsigned short>(RayTracingCommon::invalid_vertex,
                                                RayTracingCommon::invalid_vertex)
  {
  }

  ElemExtrema(const unsigned short v1, const unsigned short v2)
    : std::pair<unsigned short, unsigned short>(v1, v2)
  {
  }

  /**
   * @returns true if at the extrema (edge or vertex)
   */
  bool atExtrema() const { return first != RayTracingCommon::invalid_vertex; }
  /**
   * @returns true if the data is invalid
   */
  bool isInvalid() const
  {
    return first == RayTracingCommon::invalid_vertex && second == RayTracingCommon::invalid_vertex;
  }

  /**
   * @returns true if at a vertex
   */
  bool atVertex() const
  {
    return first != RayTracingCommon::invalid_vertex && second == RayTracingCommon::invalid_vertex;
  }
  /**
   * @returns true if at vertex \p v
   */
  bool atVertex(const unsigned short v) const
  {
    return first == v && second == RayTracingCommon::invalid_vertex;
  }

  /**
   * @returns true if at an edge
   */
  bool atEdge() const
  {
    return first != RayTracingCommon::invalid_vertex && second != RayTracingCommon::invalid_vertex;
  }
  /**
   * @returns true if at the edge defined by vertices \p v1 and \p v2
   */
  bool atEdge(const unsigned short v1, const unsigned short v2) const
  {
    return second != RayTracingCommon::invalid_vertex &&
           ((first == v1 && second == v2) || (first == v2 && second == v1));
  }

  /**
   * Invalidates the current state
   */
  void invalidate()
  {
    first = RayTracingCommon::invalid_vertex;
    second = RayTracingCommon::invalid_vertex;
  }

  /**
   * @returns The vertex ID when at a vertex
   */
  unsigned short vertex() const
  {
    mooseAssert(atVertex(), "Not at a vertex");
    return first;
  }
  /**
   * @returns The vertices that contain the edge when at an edge
   */
  const std::pair<unsigned short, unsigned short> & edgeVertices() const
  {
    mooseAssert(atEdge(), "Not at an edge");
    return *this;
  }

  /**
   * Prints the current state (at edge, at vertex, not at either)
   */
  std::string print() const;

  /**
   * Sets the "at vertex" state
   */
  void setVertex(const unsigned short vertex)
  {
    mooseAssert(vertex != RayTracingCommon::invalid_vertex, "Setting invalid vertex");
    first = vertex;
    second = RayTracingCommon::invalid_vertex;
  }
  /**
   * Sets the "at edge" state
   */
  void setEdge(const unsigned short v1, const unsigned short v2)
  {
    mooseAssert(v1 != RayTracingCommon::invalid_vertex, "Setting invalid vertex");
    mooseAssert(v2 != RayTracingCommon::invalid_vertex, "Setting invalid vertex");
    first = v1;
    second = v2;
  }
  /**
   * Sets the "at edge" state
   */
  void setEdge(const std::pair<unsigned short, unsigned short> & vertices)
  {
    setEdge(vertices.first, vertices.second);
  }

  /**
   * @returns The vertex point when at a vertex
   */
  const Point & vertexPoint(const libMesh::Elem * elem) const;
  /**
   * @returns The edge when at an edge
   */
  std::unique_ptr<const libMesh::Elem> buildEdge(const Elem * elem) const;
  /**
   * @returns Whether or not the current state (at vertex/edge) is valid
   * for the given \p elem and \p point.
   *
   * This ONLY checks for validity when atExtrema().
   */
  bool isValid(const Elem * const elem, const Point & point) const;
};

std::ostream & operator<<(std::ostream & os, const ElemExtrema & elem_extrema);
