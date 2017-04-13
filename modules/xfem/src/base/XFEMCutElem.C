/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "XFEMCutElem.h"

#include "libmesh/mesh.h"
#include "libmesh/elem.h"
#include "libmesh/node.h"
#include "libmesh/quadrature.h"
#include "libmesh/fe.h"
#include "libmesh/enum_fe_family.h"

#include "EFANode.h"
#include "EFAElement.h"

XFEMCutElem::XFEMCutElem(Elem * elem, unsigned int n_qpoints)
  : _n_nodes(elem->n_nodes()), _n_qpoints(n_qpoints), _nodes(_n_nodes, NULL), _have_weights(false)
{
  for (unsigned int i = 0; i < _n_nodes; ++i)
    _nodes[i] = elem->get_node(i);
  _elem_volume = elem->volume();
}

XFEMCutElem::~XFEMCutElem() {}

Real
XFEMCutElem::getPhysicalVolumeFraction() const
{
  return _physical_volfrac;
}

void
XFEMCutElem::getWeightMultipliers(MooseArray<Real> & weights,
                                  QBase * qrule,
                                  Xfem::XFEM_QRULE xfem_qrule,
                                  const MooseArray<Point> & q_points)
{
  if (!_have_weights)
    computeXFEMWeights(qrule, xfem_qrule, q_points);

  weights.resize(_new_weights.size());
  for (unsigned int qp = 0; qp < _new_weights.size(); ++qp)
    weights[qp] = _new_weights[qp];
}

void
XFEMCutElem::computeXFEMWeights(QBase * qrule,
                                Xfem::XFEM_QRULE xfem_qrule,
                                const MooseArray<Point> & q_points)
{
  _new_weights.clear();

  _new_weights.resize(qrule->n_points(), 1.0);

  switch (xfem_qrule)
  {
    case Xfem::VOLFRAC:
    {
      computePhysicalVolumeFraction();
      Real volfrac = getPhysicalVolumeFraction();
      for (unsigned qp = 0; qp < qrule->n_points(); ++qp)
        _new_weights[qp] = volfrac;
      break;
    }
    case Xfem::MOMENT_FITTING:
    {
      // These are the coordinates in parametric coordinates
      _qp_points = qrule->get_points();
      _qp_weights = qrule->get_weights();

      computeMomentFittingWeights();

      // Blend weights from moment fitting and volume fraction to avoid negative weights
      Real alpha = 1.0;
      if (*std::min_element(_new_weights.begin(), _new_weights.end()) < 0.0)
      {
        // One or more of the weights computed by moment fitting is negative.
        // Blend moment and volume fraction weights to keep them nonnegative.
        // Find the largest value of alpha that will keep all of the weights nonnegative.
        for (unsigned int i = 0; i < _n_qpoints; ++i)
        {
          const Real denominator = _physical_volfrac - _new_weights[i];
          if (denominator > 0.0) // Negative values would give a negative value for alpha, which
                                 // must be between 0 and 1.
          {
            const Real alpha_i = _physical_volfrac / denominator;
            if (alpha_i < alpha)
              alpha = alpha_i;
          }
        }
        for (unsigned int i = 0; i < _n_qpoints; ++i)
        {
          _new_weights[i] = alpha * _new_weights[i] + (1.0 - alpha) * _physical_volfrac;
          if (_new_weights[i] < 0.0) // We can end up with small (roundoff) negative weights
            _new_weights[i] = 0.0;
        }
      }
      break;
    }
    case Xfem::DIRECT: // remove q-points outside the partial element's physical domain
    {
      bool nonzero = false;
      for (unsigned qp = 0; qp < qrule->n_points(); ++qp)
      {
        // q_points contains quadrature point locations in physical coordinates
        if (isPointPhysical(q_points[qp]))
        {
          nonzero = true;
          _new_weights[qp] = 1.0;
        }
        else
          _new_weights[qp] = 0.0;
      }
      if (!nonzero)
      {
        // Set the weights to a small value (1e-3) to avoid having DOFs
        // with zeros on the diagonal, which occurs for nodes that are
        // connected to no physical material.
        for (unsigned qp = 0; qp < qrule->n_points(); ++qp)
          _new_weights[qp] = 1e-3;
      }
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
    if (origin2qp * normal > 0.0)
    {
      physical_flag = false; // Point outside pysical domain
      break;
    }
  }
  return physical_flag;
}
