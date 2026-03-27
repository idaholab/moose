//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMNLConvectiveHeatFluxBC.h"
#include "NLBoundaryConvectiveHeatFluxIntegrator.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMNLConvectiveHeatFluxBC);

InputParameters
MFEMNLConvectiveHeatFluxBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  params.addClassDescription(
      "Convective heat transfer boundary condition with temperature and heat "
      "transfer coefficent given by material properties to add to MFEM problems.");
  params.addParam<MFEMScalarCoefficientName>(
      "T_infinity", "0.", "Name of a coefficient specifying the far-field temperature");
  params.addParam<MFEMScalarCoefficientName>(
      "heat_transfer_coefficient",
      "1.",
      "Name of the coefficient specifying the heat transfer coefficient");
  params.addParam<MFEMScalarCoefficientName>(
      "d_heat_transfer_dT_coefficient",
      "0.",
      "Name of the coefficient specifying the derivative of the heat transfer coefficient with "
      "respect to temperature");
  return params;
}

MFEMNLConvectiveHeatFluxBC::MFEMNLConvectiveHeatFluxBC(const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _heat_transfer_coef(getScalarCoefficient("heat_transfer_coefficient")),
    _d_heat_transfer_dT_coef(getScalarCoefficient("d_heat_transfer_dT_coefficient")),
    _T_inf_coef(getScalarCoefficient("T_infinity")),
    _T_coef(getScalarCoefficientByName((getTrialVariableName())))
{
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::NonlinearFormIntegrator *
MFEMNLConvectiveHeatFluxBC::createNLIntegrator()
{
  return new Moose::MFEM::NLBoundaryConvectiveHeatFluxIntegrator(
      _heat_transfer_coef, _d_heat_transfer_dT_coef, _T_inf_coef, _T_coef);
}

#endif
