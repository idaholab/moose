/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeAxisymmetric1DSmallStrain.h"
#include "FEProblem.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<ComputeAxisymmetric1DSmallStrain>()
{
  InputParameters params = validParams<Compute1DSmallStrain>();
  params.addClassDescription("Compute a small strain in an Axisymmetric 1D problem");
  params.addCoupledVar("scalar_strain_yy", "Scalar variable scalar_strain_yy for axisymmetric 1D problem");
  params.addCoupledVar("strain_yy", "Nonlinear variable strain_yy for axisymmetric 1D problem");

  return params;
}

ComputeAxisymmetric1DSmallStrain::ComputeAxisymmetric1DSmallStrain(const InputParameters & parameters) :
    Compute1DSmallStrain(parameters),
    _scalar_strain_yy_coupled(isCoupledScalar("scalar_strain_yy")),
    _scalar_strain_yy(_scalar_strain_yy_coupled ? coupledScalarValue("scalar_strain_yy") : _zero),
    _strain_yy_coupled(isCoupled("strain_yy")),
    _strain_yy(_strain_yy_coupled ? coupledValue("strain_yy") : _zero)
{
  if (_strain_yy_coupled && _scalar_strain_yy_coupled)
    mooseError("Must define only one of strain_yy or scalar_strain_yy");
}

void
ComputeAxisymmetric1DSmallStrain::initialSetup()
{
  const auto & subdomainIDs = _mesh.meshSubdomains();
  for (auto subdomainID : subdomainIDs)
    if (_fe_problem.getCoordSystem(subdomainID) != Moose::COORD_RZ)
      mooseError("The coordinate system must be set to RZ for Axisymmetric geometries.");
}

Real
ComputeAxisymmetric1DSmallStrain::computeStrainYY()
{
  if (_scalar_strain_yy_coupled)
    return _scalar_strain_yy[0];
  else
    return _strain_yy[_qp];
}

Real
ComputeAxisymmetric1DSmallStrain::computeStrainZZ()
{
  if (!MooseUtils::relativeFuzzyEqual(_q_point[_qp](0), 0.0))
    return (*_disp[0])[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}
