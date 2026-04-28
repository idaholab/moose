//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorBoundaryFluxIntegralPostprocessor.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMVectorBoundaryFluxIntegralPostprocessor);

InputParameters
MFEMVectorBoundaryFluxIntegralPostprocessor::validParams()
{
  InputParameters params = MFEMPostprocessor::validParams();
  params += MFEMBoundaryRestrictable::validParams();
  params.addClassDescription(
      "Calculates the integral of the flux of a vector variable across a boundary.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of optional scalar coefficient to scale integrand by.");
  params.addRequiredParam<VariableName>("variable", "Name of the vector variable.");
  return params;
}

MFEMVectorBoundaryFluxIntegralPostprocessor::MFEMVectorBoundaryFluxIntegralPostprocessor(
    const InputParameters & parameters)
  : MFEMPostprocessor(parameters),
    MFEMBoundaryRestrictable(parameters,
                             *getMFEMProblem()
                                  .getProblemData()
                                  .gridfunctions.Get(getParam<VariableName>("variable"))
                                  ->ParFESpace()
                                  ->GetParMesh()),
    _var(
        getMFEMProblem().getProblemData().gridfunctions.GetRef(getParam<VariableName>("variable"))),
    _scalar_coef(getScalarCoefficient("coefficient")),
    _rt_fec(_var.ParFESpace()->GetMaxElementOrder(), getMesh().Dimension()),
    _rt_vector_fespace(const_cast<mfem::ParMesh *>(&getMesh()), &_rt_fec),
    _rt_var(&_rt_vector_fespace),
    _var_coef(&_var),
    _boundary_integrator(&_rt_vector_fespace)
{
  _boundary_integrator.AddBoundaryIntegrator(
      new mfem::VectorFEBoundaryFluxLFIntegrator(_scalar_coef), getBoundaryMarkers());
}

void
MFEMVectorBoundaryFluxIntegralPostprocessor::execute()
{
  _rt_var.ProjectBdrCoefficientNormal(_var_coef, getBoundaryMarkers());
  _boundary_integrator.Assemble();
  _integral = _boundary_integrator(_rt_var);
}

PostprocessorValue
MFEMVectorBoundaryFluxIntegralPostprocessor::getValue() const
{
  return _integral;
}

#endif
