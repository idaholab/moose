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

#include "XFEMElementPairLocator.h"

XFEMElementPairLocator::XFEMElementPairLocator(MooseSharedPointer<XFEM> xfem, unsigned int interface_id) :
    ElementPairLocator(interface_id),
    _xfem(xfem)
{
}

XFEMElementPairLocator::~XFEMElementPairLocator()
{
}

void 
XFEMElementPairLocator::reinit()
{
  _element_pair_info.clear();
  _elem_pairs.clear();

  _xfem->getXFEMCutElemPair(_elem_pairs);

  for (unsigned int ie = 0; ie < _elem_pairs.size(); ++ie)
  {
    const Elem * elem = _elem_pairs[ie].first;

    std::vector<Point> intersectionPoints;

    Point normal(0.0, 0.0, 0.0);
    std::vector<Point> q_point;
    std::vector<Real> JxW;
   
    // for element
    _xfem->getXFEMIntersectionInfo(elem, 0, normal, intersectionPoints);

    if (intersectionPoints.size() == 2)
      _xfem->getXFEMqRuleOnLine(intersectionPoints, q_point, JxW);
    else
      _xfem->getXFEMqRuleOnSurface(intersectionPoints, q_point, JxW);

    ElementPairInfo * elem_info = new ElementPairInfo(elem, q_point, JxW, normal);     

    _element_pair_info[elem] = elem_info;
  }
}

