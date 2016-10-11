/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputePlaneFiniteStrain.h"

template<>
InputParameters validParams<ComputePlaneFiniteStrain>()
{
  InputParameters params = validParams<Compute2DFiniteStrain>();
  params.addClassDescription("Compute strain increment and rotation increment for finite strain under 2D planar assumptions.");
  params.addCoupledVar("scalar_strain_zz", "Variable containing the out-of-plane strain");
  return params;
}

ComputePlaneFiniteStrain::ComputePlaneFiniteStrain(const InputParameters & parameters) :
    Compute2DFiniteStrain(parameters),
    _scalar_strain_zz(isCoupledScalar("scalar_strain_zz") ? coupledScalarValue("scalar_strain_zz") : _zero),
    _scalar_strain_zz_old(isCoupledScalar("scalar_strain_zz") ? coupledScalarValueOld("scalar_strain_zz") : _zero)
{
}

Real
ComputePlaneFiniteStrain::computeGradDispZZ()
{
  /**
   * This is consistent with the approximation of stretch rate tensor
   * D = log(sqrt(Fhat^T * Fhat)) / dt
   */
    return std::exp(_scalar_strain_zz[0]) - 1.0;
}

Real
ComputePlaneFiniteStrain::computeGradDispZZold()
{
    return std::exp(_scalar_strain_zz_old[0]) - 1.0;
}
