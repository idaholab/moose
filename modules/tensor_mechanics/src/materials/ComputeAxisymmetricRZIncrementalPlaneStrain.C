/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeAxisymmetricRZIncrementalPlaneStrain.h"
#include "Assembly.h"

template<>
InputParameters validParams<ComputeAxisymmetricRZIncrementalPlaneStrain>()
{
  InputParameters params = validParams<Compute1DIncrementalStrain>();
  params.addClassDescription("Compute a strain increment and rotation increment for finite strains under axisymmetric assumptions.");
  params.addCoupledVar("strain_yy", "The yy strain");
  params.addCoupledVar("scalar_strain_yy", "The yy strain (scalar variable)");
  return params;
}

ComputeAxisymmetricRZIncrementalPlaneStrain::ComputeAxisymmetricRZIncrementalPlaneStrain(const InputParameters & parameters) :
    Compute1DIncrementalStrain(parameters),
    _disp_old_0(coupledValueOld("displacements", 0)),
    _have_strain_yy(isCoupled("strain_yy")),
    _strain_yy(_have_strain_yy ? coupledValue("strain_yy") : _zero),
    _strain_yy_old(_have_strain_yy ? coupledValueOld("strain_yy") : _zero),
    _have_scalar_strain_yy(isCoupledScalar("scalar_strain_yy")),
    _scalar_strain_yy(_have_scalar_strain_yy ? coupledScalarValue("scalar_strain_yy") : _zero),
    _scalar_strain_yy_old(_have_scalar_strain_yy ? coupledScalarValueOld("scalar_strain_yy") : _zero)
{
  if (_have_strain_yy && _have_scalar_strain_yy)
      mooseError("Must define only one of strain_yy or scalar_strain_yy");
}

void
ComputeAxisymmetricRZIncrementalPlaneStrain::initialSetup()
{
  if (_assembly.coordSystem() != Moose::COORD_RZ)
    mooseError("The coordinate system must be set to RZ for Axisymmetric simulations.");
}

Real
ComputeAxisymmetricRZIncrementalPlaneStrain::computeDUYDY()
{
  if (_have_strain_yy)
    return _strain_yy[_qp];
  else if (_have_scalar_strain_yy && _scalar_strain_yy.size()>0)
    return _scalar_strain_yy[0];
  else
    return 0;
}

Real
ComputeAxisymmetricRZIncrementalPlaneStrain::computeDUYDYOld()
{
  if (_have_strain_yy)
    return _strain_yy_old[_qp];
  else if (_have_scalar_strain_yy && _scalar_strain_yy_old.size()>0)
    return _scalar_strain_yy_old[0];
  else
    return 0;
}

Real
ComputeAxisymmetricRZIncrementalPlaneStrain::computeDUZDZ()
{
  if (!MooseUtils::relativeFuzzyEqual(_q_point[_qp](0), 0.0))
    return (*_disp[0])[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}

Real
ComputeAxisymmetricRZIncrementalPlaneStrain::computeDUZDZOld()
{
  if (!MooseUtils::relativeFuzzyEqual(_q_point[_qp](0), 0.0))
    return _disp_old_0[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}
