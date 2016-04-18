/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "XFEMElementPairLocator.h"

XFEMElementPairLocator::XFEMElementPairLocator(MooseSharedPointer<XFEM> xfem, unsigned int interface_id) :
    ElementPairLocator(interface_id),
    _xfem(xfem)
{
  _elem_pairs = _xfem->getXFEMCutElemPairs();
}

void
XFEMElementPairLocator::reinit()
{
  _element_pair_info.clear();

  for (std::list<std::pair<const Elem *, const Elem*> >::const_iterator it = _elem_pairs->begin();
       it != _elem_pairs->end(); ++it)
  {
    const Elem * elem = it->first;
    const Elem * elem2 = it->second;

    std::vector<Point> intersectionPoints;

    Point normal;
    std::vector<Point> q_points;
    std::vector<Real> weights;

    unsigned int plane_id = 0; // Only support one cut plane for the time being
    _xfem->getXFEMIntersectionInfo(elem, plane_id, normal, intersectionPoints);

    if (intersectionPoints.size() == 2)
      _xfem->getXFEMqRuleOnLine(intersectionPoints, q_points, weights);
    else if (intersectionPoints.size() > 2)
      _xfem->getXFEMqRuleOnSurface(intersectionPoints, q_points, weights);

    ElementPairInfo new_elem_info(elem, q_points, weights, normal);
    _element_pair_info.insert(std::pair<const Elem*, ElementPairInfo>(elem, new_elem_info));
  }
}
