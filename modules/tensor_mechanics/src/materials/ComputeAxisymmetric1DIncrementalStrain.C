/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeAxisymmetric1DIncrementalStrain.h"
#include "Assembly.h"

template<>
InputParameters validParams<ComputeAxisymmetric1DIncrementalStrain>()
{
  InputParameters params = validParams<Compute1DIncrementalStrain>();
  params.addClassDescription("Compute strain increment for small strains in an axisymmetric 1D problem");
  params.addCoupledVar("scalar_strain_yy", "Scalar variable scalar_strain_yy for axisymmetric 1D problem");
  params.addCoupledVar("strain_yy", "Nonlinear variable strain_yy for axisymmetric 1D problem");

  return params;
}

ComputeAxisymmetric1DIncrementalStrain::ComputeAxisymmetric1DIncrementalStrain(const InputParameters & parameters) :
    Compute1DIncrementalStrain(parameters),
    _disp_old_0(coupledValueOld("displacements", 0)),
    _strain_yy_coupled(isCoupled("strain_yy")),
    _strain_yy(_strain_yy_coupled ? coupledValue("strain_yy") : _zero),
    _strain_yy_old(_strain_yy_coupled ? coupledValueOld("strain_yy") : _zero),
    _scalar_strain_yy_coupled(isCoupledScalar("scalar_strain_yy")),
    _scalar_strain_yy(_scalar_strain_yy_coupled ? coupledScalarValue("scalar_strain_yy") : _zero),
    _scalar_strain_yy_old(_scalar_strain_yy_coupled ? coupledScalarValueOld("scalar_strain_yy") : _zero)
{
  if (_strain_yy_coupled && _scalar_strain_yy_coupled)
    mooseError("Must define only one of strain_yy or scalar_strain_yy");
}

void
ComputeAxisymmetric1DIncrementalStrain::initialSetup()
{
  if (_assembly.coordSystem() != Moose::COORD_RZ)
    mooseError("The coordinate system must be set to RZ for Axisymmetric 1D simulations");
}

Real
ComputeAxisymmetric1DIncrementalStrain::computeGradDispYY()
{
  if (_scalar_strain_yy_coupled)
    return _scalar_strain_yy[0];
  else
    return _strain_yy[_qp];
}

Real
ComputeAxisymmetric1DIncrementalStrain::computeGradDispYYOld()
{
  if (_scalar_strain_yy_coupled)
    return _scalar_strain_yy_old[0];
  else
    return _strain_yy_old[_qp];
}

Real
ComputeAxisymmetric1DIncrementalStrain::computeGradDispZZ()
{
  if (!MooseUtils::relativeFuzzyEqual(_q_point[_qp](0), 0.0))
    return (*_disp[0])[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}

Real
ComputeAxisymmetric1DIncrementalStrain::computeGradDispZZOld()
{
  if (!MooseUtils::relativeFuzzyEqual(_q_point[_qp](0), 0.0))
    return _disp_old_0[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}
