//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XFEMElementPairLocator.h"

XFEMElementPairLocator::XFEMElementPairLocator(std::shared_ptr<XFEM> xfem,
                                               unsigned int interface_id,
                                               bool use_displaced_mesh)
  : ElementPairLocator(interface_id), _xfem(xfem), _use_displaced_mesh(use_displaced_mesh)
{
  if (_use_displaced_mesh)
    _elem_pairs = _xfem->getXFEMDisplacedCutElemPairs(interface_id);
  else
    _elem_pairs = _xfem->getXFEMCutElemPairs(interface_id);
}

void
XFEMElementPairLocator::reinit()
{
  // Does not support secondary cut yet.
  if (_xfem->has_secondary_cut())
    return;

  _element_pair_info.clear();

  for (std::list<std::pair<const Elem *, const Elem *>>::const_iterator it = _elem_pairs->begin();
       it != _elem_pairs->end();
       ++it)
  {
    const Elem * elem1 = it->first;
    const Elem * elem2 = it->second;

    std::vector<Point> intersectionPoints1;
    Point normal1;
    std::vector<Point> q_points1;
    std::vector<Real> weights1;

    unsigned int plane_id = 0; // Only support one cut plane for the time being

    _xfem->getXFEMIntersectionInfo(
        elem1, plane_id, normal1, intersectionPoints1, _use_displaced_mesh);

    if (intersectionPoints1.size() == 2)
      _xfem->getXFEMqRuleOnLine(intersectionPoints1, q_points1, weights1);
    else if (intersectionPoints1.size() > 2)
      _xfem->getXFEMqRuleOnSurface(intersectionPoints1, q_points1, weights1);

    if (!_use_displaced_mesh)
    {
      ElementPairInfo new_elem_info(
          elem1, elem2, q_points1, q_points1, weights1, weights1, normal1, -normal1);
      _element_pair_info.insert(
          std::pair<std::pair<const Elem *, const Elem *>, ElementPairInfo>(*it, new_elem_info));
    }
    else
    {
      std::vector<Point> intersectionPoints2;
      Point normal2;
      std::vector<Point> q_points2;
      std::vector<Real> weights2;

      _xfem->getXFEMIntersectionInfo(
          elem2, plane_id, normal2, intersectionPoints2, _use_displaced_mesh);

      // reverse the order of intersectionPoints2
      std::reverse(std::begin(intersectionPoints2), std::end(intersectionPoints2));

      if (intersectionPoints2.size() == 2)
        _xfem->getXFEMqRuleOnLine(intersectionPoints2, q_points2, weights2);
      else if (intersectionPoints2.size() > 2)
        _xfem->getXFEMqRuleOnSurface(intersectionPoints2, q_points2, weights2);

      ElementPairInfo new_elem_info(
          elem1, elem2, q_points1, q_points2, weights1, weights2, normal1, normal2);
      _element_pair_info.insert(
          std::pair<std::pair<const Elem *, const Elem *>, ElementPairInfo>(*it, new_elem_info));
    }
  }
}

void
XFEMElementPairLocator::update()
{
  reinit();
}
