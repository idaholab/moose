/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputePlaneIncrementalStrain.h"

template<>
InputParameters validParams<ComputePlaneIncrementalStrain>()
{
  InputParameters params = validParams<Compute2DIncrementalStrain>();
  params.addClassDescription("Compute strain increment for small strain under 2D planar assumptions.");
  params.addCoupledVar("scalar_strain_zz", "Scalar variable containing the out-of-plane strain for generalized plane strain");
  params.addCoupledVar("strain_zz", "Nonlinear variable containing the out-of-plane strain for plane stress");

  return params;
}

ComputePlaneIncrementalStrain::ComputePlaneIncrementalStrain(const InputParameters & parameters) :
    Compute2DIncrementalStrain(parameters),
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
ComputePlaneIncrementalStrain::computeGradDispZZ()
{
  if (_scalar_strain_zz_coupled)
    return _scalar_strain_zz[0];
  else
    return _strain_zz[_qp];
}

Real
ComputePlaneIncrementalStrain::computeGradDispZZOld()
{
  if (_scalar_strain_zz_coupled)
    return _scalar_strain_zz_old[0];
  else
    return _strain_zz_old[_qp];
}
