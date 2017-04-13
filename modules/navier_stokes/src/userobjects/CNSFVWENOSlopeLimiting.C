/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVWENOSlopeLimiting.h"

template <>
InputParameters
validParams<CNSFVWENOSlopeLimiting>()
{
  InputParameters params = validParams<SlopeLimitingBase>();

  params.addClassDescription("A user object that performs WENO slope limiting to get the limited "
                             "slopes of cell average variables in multi-dimensions.");

  params.addParam<Real>("linear_weight", 1, "Linear weight, default = 1");

  params.addParam<Real>("decay_power", 1, "Decay power, default = 1");

  return params;
}

CNSFVWENOSlopeLimiting::CNSFVWENOSlopeLimiting(const InputParameters & parameters)
  : SlopeLimitingBase(parameters),
    _lweig(getParam<Real>("linear_weight")),
    _dpowe(getParam<Real>("decay_power"))
{
}

std::vector<RealGradient>
CNSFVWENOSlopeLimiting::limitElementSlope() const
{
  const Elem * elem = _current_elem;

  /// current element id
  dof_id_type _elementID = elem->id();

  /// number of conserved variables
  unsigned int nvars = 5;

  /// number of sides surrounding an element
  unsigned int nside = elem->n_sides();

  /// vector for the limited gradients of the conserved variables
  std::vector<RealGradient> ugrad(nvars, RealGradient(0., 0., 0.));

  /// initialize local cache for reconstructed element gradients
  std::vector<RealGradient> rugrad(nvars, RealGradient(0., 0., 0.));

  /// a small number to avoid dividing by zero
  Real eps = 1e-7;

  /// initialize oscillation indicator
  std::vector<std::vector<Real>> osci(nside + 1, std::vector<Real>(nvars, 0.));

  /// initialize weight coefficients
  std::vector<std::vector<Real>> weig(nside + 1, std::vector<Real>(nvars, 0.));

  /// initialize summed coefficients
  std::vector<Real> summ(nvars, 0.);

  /// compute oscillation indicators, nonlinear weights and their summ for each stencil

  rugrad = _rslope.getElementSlope(_elementID);

  for (unsigned int iv = 0; iv < nvars; iv++)
  {
    osci[0][iv] = std::sqrt(rugrad[iv] * rugrad[iv]);

    weig[0][iv] = _lweig / std::pow(eps + osci[0][iv], _dpowe);

    summ[iv] = weig[0][iv];
  }

  for (unsigned int is = 0; is < nside; is++)
  {
    unsigned int in = is + 1;

    if (elem->neighbor(is) != NULL)
    {
      dof_id_type _neighborID = elem->neighbor(is)->id();
      rugrad = _rslope.getElementSlope(_neighborID);

      for (unsigned int iv = 0; iv < nvars; iv++)
      {
        osci[in][iv] = std::sqrt(rugrad[iv] * rugrad[iv]);

        weig[in][iv] = 1. / std::pow(eps + osci[in][iv], _dpowe);

        summ[iv] += weig[in][iv];
      }
    }
  }

  /// compute the normalized weight

  for (unsigned int iv = 0; iv < nvars; iv++)
    weig[0][iv] = weig[0][iv] / summ[iv];

  for (unsigned int is = 0; is < nside; is++)
  {
    unsigned int in = is + 1;

    if (elem->neighbor(is) != NULL)
      for (unsigned int iv = 0; iv < nvars; iv++)
        weig[in][iv] = weig[in][iv] / summ[iv];
  }

  /// compute the limited slope

  rugrad = _rslope.getElementSlope(_elementID);

  for (unsigned int iv = 0; iv < nvars; iv++)
    ugrad[iv] = weig[0][iv] * rugrad[iv];

  for (unsigned int is = 0; is < nside; is++)
  {
    unsigned int in = is + 1;

    if (elem->neighbor(is) != NULL)
    {
      dof_id_type _neighborID = elem->neighbor(is)->id();
      rugrad = _rslope.getElementSlope(_neighborID);

      for (unsigned int iv = 0; iv < nvars; iv++)
        ugrad[iv] += weig[in][iv] * rugrad[iv];
    }
  }

  return ugrad;
}
