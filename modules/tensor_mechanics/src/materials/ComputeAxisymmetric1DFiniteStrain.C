/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeAxisymmetric1DFiniteStrain.h"
#include "Assembly.h"
#include "FEProblem.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<ComputeAxisymmetric1DFiniteStrain>()
{
  InputParameters params = validParams<Compute1DFiniteStrain>();
  params.addClassDescription("Compute a strain increment and rotation increment for finite strains in an axisymmetric 1D problem");
  params.addCoupledVar("scalar_strain_yy", "Scalar variable scalar_strain_yy for axisymmetric 1D problem");
  params.addCoupledVar("strain_yy", "Nonlinear variable strain_yy for axisymmetric 1D problem");

  return params;
}

ComputeAxisymmetric1DFiniteStrain::ComputeAxisymmetric1DFiniteStrain(const InputParameters & parameters) :
    Compute1DFiniteStrain(parameters),
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
ComputeAxisymmetric1DFiniteStrain::initialSetup()
{
  const auto & subdomainIDs = _mesh.meshSubdomains();
  for (auto subdomainID : subdomainIDs)
    if (_fe_problem.getCoordSystem(subdomainID) != Moose::COORD_RZ)
      mooseError("The coordinate system must be set to RZ for Axisymmetric simulations.");
}

Real
ComputeAxisymmetric1DFiniteStrain::computeGradDispYY()
{
  if (_scalar_strain_yy_coupled)
    return std::exp(_scalar_strain_yy[0]) - 1.0;
  else
    return std::exp(_strain_yy[_qp]) - 1.0;
}

Real
ComputeAxisymmetric1DFiniteStrain::computeGradDispYYOld()
{
  if (_scalar_strain_yy_coupled)
    return std::exp(_scalar_strain_yy_old[0]) - 1.0;
  else
    return std::exp(_strain_yy_old[_qp]) - 1.0;
}

Real
ComputeAxisymmetric1DFiniteStrain::computeGradDispZZ()
{
  if (!MooseUtils::relativeFuzzyEqual(_q_point[_qp](0), 0.0))
    return (*_disp[0])[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}

Real
ComputeAxisymmetric1DFiniteStrain::computeGradDispZZOld()
{
  if (!MooseUtils::relativeFuzzyEqual(_q_point[_qp](0), 0.0))
    return _disp_old_0[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}
