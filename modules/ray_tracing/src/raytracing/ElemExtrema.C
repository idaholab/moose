//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElemExtrema.h"

// libMesh includes
#include "libmesh/elem.h"

const Point &
ElemExtrema::vertexPoint(const libMesh::Elem * elem) const
{
  return elem->point(vertex());
}

std::unique_ptr<const libMesh::Elem>
ElemExtrema::buildEdge(const libMesh::Elem * elem) const
{
  mooseAssert(atEdge(), "Did not intersect edge");

  for (const auto e : elem->edge_index_range())
    if (elem->is_node_on_edge(first, e) && elem->is_node_on_edge(second, e))
      return elem->build_edge_ptr(e);

  mooseError("Element does not contain vertices");
}

std::string
ElemExtrema::print() const
{
  std::stringstream oss;
  if (atVertex())
    oss << "at vertex " << vertex();
  else if (atEdge())
    oss << "at edge with vertices " << first << " and " << second;
  else
    oss << "not at extrema";
  return oss.str();
}

bool
ElemExtrema::isValid(const Elem * const elem, const Point & point) const
{
  if (atVertex())
    return vertexPoint(elem).absolute_fuzzy_equals(point);
  if (elem->dim() == 3 && atEdge())
    return buildEdge(elem)->contains_point(point);

  return true;
}

std::ostream &
operator<<(std::ostream & os, const ElemExtrema & elem_extrema)
{
  os << elem_extrema.print();
  return os;
}
