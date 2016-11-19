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
  params.addCoupledVar("scalar_strain", "Scalar variable for axisymmetric 1D problem");
  params.addCoupledVar("variable_strain", "Nonlinear variable for axisymmetric 1D problem");

  return params;
}

ComputeAxisymmetric1DSmallStrain::ComputeAxisymmetric1DSmallStrain(const InputParameters & parameters) :
    Compute1DSmallStrain(parameters),
    _scalar_strain_coupled(isCoupledScalar("scalar_strain")),
    _scalar_strain(_scalar_strain_coupled ? coupledScalarValue("scalar_strain") : _zero),
    _variable_strain_coupled(isCoupled("variable_strain")),
    _variable_strain(_variable_strain_coupled ? coupledValue("variable_strain") : _zero)
{
  if (_variable_strain_coupled && _scalar_strain_coupled)
    mooseError("Must define only one of variable_strain or scalar_strain");
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
  if (_scalar_strain_coupled)
    return _scalar_strain[0];
  else
    return _variable_strain[_qp];
}

Real
ComputeAxisymmetric1DSmallStrain::computeStrainZZ()
{
  if (!MooseUtils::relativeFuzzyEqual(_q_point[_qp](0), 0.0))
    return (*_disp[0])[_qp] / _q_point[_qp](0);
  else
    return 0.0;
}
