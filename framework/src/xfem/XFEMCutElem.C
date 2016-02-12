/***************************************************************/
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

#include "XFEMCutElem.h"

#include "libmesh/mesh.h"
#include "libmesh/elem.h"
#include "libmesh/node.h"
#include "libmesh/quadrature.h"

#include "EFANode.h"
#include "EFAElement.h"

XFEMCutElem::XFEMCutElem(Elem* elem, unsigned int n_qpoints):
  _n_nodes(elem->n_nodes()),
  _n_qpoints(n_qpoints),
  _nodes(_n_nodes,NULL),
  _have_weights(false)
{
  for (unsigned int i = 0; i < _n_nodes; ++i)
    _nodes[i] = elem->get_node(i);
  _elem_volume = elem->volume();
}

XFEMCutElem::~XFEMCutElem()
{
}

Real
XFEMCutElem::getPhysicalVolumeFraction() const
{
  return _physical_volfrac;
}

void
XFEMCutElem::getWeightMultipliers(MooseArray<Real> & weights, QBase * qrule, XFEM_QRULE xfem_qrule)
{
  if (!_have_weights)
    computeXFEMWeights(qrule, xfem_qrule);

  weights.resize(_new_weights.size());
  for (unsigned int qp=0; qp<_new_weights.size(); ++qp)
    weights[qp] = _new_weights[qp];
}

void
XFEMCutElem::computeXFEMWeights(QBase * qrule, XFEM_QRULE xfem_qrule)
{
  _new_weights.clear();

  _new_weights.resize(qrule->n_points(), 1.0);

  switch (xfem_qrule)
  {
    case VOLFRAC:
    {
      computePhysicalVolumeFraction();
      Real volfrac = getPhysicalVolumeFraction();
      for (unsigned qp = 0; qp < qrule->n_points(); qp++)
      {
        _new_weights[qp] = volfrac;
      }
      break;
    }
    case MOMENT_FITTING:
    {
      _qp_points = qrule->get_points();
      _qp_weights = qrule->get_weights();

      computeMomentFittingWeights();
      break;
    }
    case DIRECT: // remove q-points outside the partial element's physical domain
    {
      mooseError("The DIRECT option for XFEM_QRULE is not yet implemented");
//      std::vector<Point> qp_points = qrule->get_points();
//      for (unsigned qp = 0; qp < qrule->n_points(); qp++)
//      {
//        //TODO: this is wrong.  We need physical coords
//        if (isPointPhysical(qp_points[qp]))
//          _new_weights[qp] = 1.0;
//        else
//          _new_weights[qp] = 0.0;
//      }
      break;
    }
    default:
      mooseError("Undefined option for XFEM_QRULE");
  }
  _have_weights = true;
}

bool
XFEMCutElem::isPointPhysical(const Point & p) const
{
  // determine whether the point is inside the physical domain of a partial element
  bool physical_flag = true;
  unsigned int n_cut_planes = numCutPlanes();
  for (unsigned int plane_id = 0; plane_id < n_cut_planes; ++plane_id)
  {
    Point origin = getCutPlaneOrigin(plane_id);
    Point normal = getCutPlaneNormal(plane_id);
    Point origin2qp = p - origin;
    if (origin2qp*normal > 0.0)
    {
      physical_flag = false; // Point outside pysical domain
      break;
    }
  }
  return physical_flag;
}

