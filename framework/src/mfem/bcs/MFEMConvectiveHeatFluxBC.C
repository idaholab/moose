//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMConvectiveHeatFluxBC.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMConvectiveHeatFluxBC);

InputParameters
MFEMConvectiveHeatFluxBC::validParams()
{
  InputParameters params = MFEMIntegratedBC::validParams();
  // FIXME: Should these really be specified via properties? T_infinity in particular? Use functions
  // instead?
  params.addClassDescription(
      "Convective heat transfer boundary condition with temperature and heat "
      "transfer coefficent given by material properties to add to MFEM problems.");
  params.addParam<MFEMScalarCoefficientName>(
      "T_infinity", "0.", "Name of a coefficient specifying the far-field temperature");
  params.addParam<MFEMScalarCoefficientName>(
      "heat_transfer_coefficient",
      "1.",
      "Name of the coefficient specifying the heat transfer coefficient");
  return params;
}

MFEMConvectiveHeatFluxBC::MFEMConvectiveHeatFluxBC(const InputParameters & parameters)
  : MFEMIntegratedBC(parameters),
    _heat_transfer_coef(
        getScalarCoefficient(getParam<MFEMScalarCoefficientName>("heat_transfer_coefficient"))),
    _T_inf_coef(getScalarCoefficient(getParam<MFEMScalarCoefficientName>("T_infinity"))),
    _external_heat_flux_coef(
        getMFEMProblem().getCoefficients().declareScalar<mfem::ProductCoefficient>(
            "__ConvectiveHeatFluxBC_" + parameters.get<std::string>("_unique_name"),
            _heat_transfer_coef,
            _T_inf_coef))
{
}

// Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
// caller.
mfem::LinearFormIntegrator *
MFEMConvectiveHeatFluxBC::createLFIntegrator()
{
  return new mfem::BoundaryLFIntegrator(_external_heat_flux_coef);
}

// Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
mfem::BilinearFormIntegrator *
MFEMConvectiveHeatFluxBC::createBFIntegrator()
{
  return new mfem::BoundaryMassIntegrator(_heat_transfer_coef);
}

#endif
