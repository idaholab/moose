/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWVANGENUCHTEN_H
#define POROUSFLOWVANGENUCHTEN_H

#include "MooseTypes.h"

/**
 * Utility functions for van-genuchten effective saturation,
 * as a function of porepressure (not capillary pressure),
 * and first and second derivs wrt porepressure.
 * So seff = 1 for p >= 0
 *    seff = (1 + (-al*p)^(1/(1-m)))^(-m) < 1 for p < 0
 *
 * Also, van-genuchten relative permeability as a function
 * of saturation
 */
namespace PorousFlowVanGenuchten
{
/**
 * effective saturation as a fcn of porepressure
 * @param p porepressure
 * @param al van-genuchten alpha parameter
 * @param m van-genuchten m parameter
 */
Real seff(Real p, Real al, Real m);

/**
 * derivative of effective saturation wrt porepressure
 * @param p porepressure
 * @param al van-genuchten alpha parameter
 * @param m van-genuchten m parameter
 */
Real dseff(Real p, Real al, Real m);

/**
 * 2nd derivative of effective saturation wrt porepressure
 * @param p porepressure
 * @param al van-genuchten alpha parameter
 * @param m van-genuchten m parameter
 */
Real d2seff(Real p, Real al, Real m);

/**
 * Relative permeability as a function of effective saturation
 * @param seff effective saturation
 * @param m Van-Genuchten m index
 */
Real relativePermeability(Real seff, Real m);

/**
 * Derivative of relative permeability with respect to effective saturation
 * @param seff effective saturation
 * @param m Van-Genuchten m index
 */
Real drelativePermeability(Real seff, Real m);

/**
 * Second derivative of relative permeability with respect to effective saturation
 * @param seff effective saturation
 * @param m Van-Genuchten m index
 */
Real d2relativePermeability(Real seff, Real m);
}

#endif // POROUSFLOWVANGENUCHTEN_H
