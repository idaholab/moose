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
  params.addCoupledVar("scalar_strain_zz", "Scalar variable containing the out-of-plane strain for generalized plane strain");
  params.addCoupledVar("strain_zz", "Nonlinear variable containing the out-of-plane strain for plane stress");

  return params;
}

ComputePlaneFiniteStrain::ComputePlaneFiniteStrain(const InputParameters & parameters) :
    Compute2DFiniteStrain(parameters),
    _scalar_strain_zz_coupled(isCoupledScalar("scalar_strain_zz")),
    _scalar_strain_zz(_scalar_strain_zz_coupled ? coupledScalarValue("scalar_strain_zz") : _zero),
    _scalar_strain_zz_old(_scalar_strain_zz_coupled ? coupledScalarValueOld("scalar_strain_zz") : _zero),
    _strain_zz_coupled(isCoupled("strain_zz")),
    _strain_zz(_strain_zz_coupled ? coupledValue("strain_zz") : _zero),
    _strain_zz_old(_strain_zz_coupled ? coupledValueOld("strain_zz") : _zero)
{
  if (_strain_zz_coupled && _scalar_strain_zz_coupled)
    mooseError("Must define only one of strain_zz or scalar_strain_zz");
}

Real
ComputePlaneFiniteStrain::computeGradDispZZ()
{
  /**
   * This is consistent with the approximation of stretch rate tensor
   * D = log(sqrt(Fhat^T * Fhat)) / dt
   */
  if (_scalar_strain_zz_coupled)
    return std::exp(_scalar_strain_zz[0]) - 1.0;
  else
    return std::exp(_strain_zz[_qp]) - 1.0;
}

Real
ComputePlaneFiniteStrain::computeGradDispZZOld()
{
  if (_scalar_strain_zz_coupled)
    return std::exp(_scalar_strain_zz_old[0]) - 1.0;
  else
    return std::exp(_strain_zz_old[_qp]) - 1.0;
}
