/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVMinmaxSlopeLimiting.h"

template <>
InputParameters
validParams<CNSFVMinmaxSlopeLimiting>()
{
  InputParameters params = validParams<SlopeLimitingBase>();
  params.addClassDescription("A user object that performs the min-max slope limiting to get the "
                             "limited slopes of cell average variables.");
  return params;
}

CNSFVMinmaxSlopeLimiting::CNSFVMinmaxSlopeLimiting(const InputParameters & parameters)
  : SlopeLimitingBase(parameters)
{
}

std::vector<RealGradient>
CNSFVMinmaxSlopeLimiting::limitElementSlope() const
{
  const Elem * elem = _current_elem;

  /// current element id
  dof_id_type _elementID = elem->id();

  /// current element centroid coordinte
  Point ecent = elem->centroid();

  /// number of sides surrounding an element
  unsigned int nside = elem->n_sides();

  /// number of conserved variables
  unsigned int nvars = 5;

  /// vector for the centroid of the sides surrounding an element
  std::vector<Point> scent(nside, Point(0., 0., 0.));

  /// vector for the limited gradients of the conserved variables
  std::vector<RealGradient> ugrad(nvars, RealGradient(0., 0., 0.));

  /// a flag to indicate the boundary side of the current cell
  bool bflag = false;

  /// initialize local minimum variable values
  std::vector<Real> umin(nvars, 0.);
  /// initialize local maximum variable values
  std::vector<Real> umax(nvars, 0.);
  /// initialize local cache for element variable values
  std::vector<Real> uelem(nvars, 0.);
  /// initialize local cache for reconstructed element gradients
  std::vector<RealGradient> rugrad(nvars, RealGradient(0., 0., 0.));
  /// initialize local cache for element slope limitor values
  std::vector<std::vector<Real>> limit(nside + 1, std::vector<Real>(nvars, 1.));

  Real dij = 0.;

  uelem = _rslope.getElementAverageValue(_elementID);

  for (unsigned int iv = 0; iv < nvars; iv++)
  {
    umin[iv] = uelem[iv];
    umax[iv] = uelem[iv];
  }

  /// loop over the sides to find the min and max cell-average values

  for (unsigned int is = 0; is < nside; is++)
  {
    if (elem->neighbor(is) != NULL)
    {
      dof_id_type _neighborID = elem->neighbor(is)->id();
      uelem = _rslope.getElementAverageValue(_neighborID);
      scent[is] = _rslope.getSideCentroid(_elementID, _neighborID);
    }
    else
    {
      uelem = _rslope.getBoundaryAverageValue(_elementID, is);
      scent[is] = _rslope.getBoundarySideCentroid(_elementID, is);
    }

    for (unsigned int iv = 0; iv < nvars; iv++)
    {
      if (uelem[iv] < umin[iv])
        umin[iv] = uelem[iv];
      if (uelem[iv] > umax[iv])
        umax[iv] = uelem[iv];
    }
  }

  /// loop over the sides to compute the limiter values

  rugrad = _rslope.getElementSlope(_elementID);
  uelem = _rslope.getElementAverageValue(_elementID);

  for (unsigned int is = 0; is < nside; is++)
  {
    for (unsigned int iv = 0; iv < nvars; iv++)
    {
      dij = (scent[is] - ecent) * rugrad[iv];

      if (dij > 0.)
        limit[is + 1][iv] = std::min(1., (umax[iv] - uelem[iv]) / dij);
      else if (dij < 0.)
        limit[is + 1][iv] = std::min(1., (umin[iv] - uelem[iv]) / dij);
    }
  }

  /// loop over the sides to get the final limiter of this cell

  for (unsigned int is = 0; is < nside; is++)
  {
    for (unsigned int iv = 0; iv < nvars; iv++)
    {
      if (limit[0][iv] > limit[is + 1][iv])
        limit[0][iv] = limit[is + 1][iv];
    }
  }

  /// get the limited slope of this cell

  if (!_include_bc && bflag)
  {
    for (unsigned int iv = 0; iv < nvars; iv++)
      ugrad[iv] = RealGradient(0., 0., 0.);
  }
  else
  {
    for (unsigned int iv = 0; iv < nvars; iv++)
      ugrad[iv] = rugrad[iv] * limit[0][iv];
  }

  return ugrad;
}
