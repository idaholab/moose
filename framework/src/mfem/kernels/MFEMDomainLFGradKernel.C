//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMDomainLFGradKernel.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMDomainLFGradKernel);

InputParameters
MFEMDomainLFGradKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription("Adds the domain integrator to an MFEM problem for the linear form "
                             "$(f, v)_\\Omega$ "
                             "arising from the weak form of the forcing term $f$.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "The name of the scalar coefficient f");
  return params;
}

MFEMDomainLFGradKernel::MFEMDomainLFGradKernel(const InputParameters & parameters)
  : MFEMKernel(parameters), _coef(getScalarCoefficient("coefficient"))
{
  // declares GradientGridFunctionCoefficient
  getMFEMProblem().getCoefficients().declareVector<mfem::GradientGridFunctionCoefficient>(
      name(), getMFEMProblem().getProblemData().gridfunctions.Get(_test_var_name));
}

mfem::LinearFormIntegrator *
MFEMDomainLFGradKernel::createNLAIntegrator()
{
  mfem::VectorCoefficient & vec_coef =
      getMFEMProblem().getCoefficients().getVectorCoefficient(name());
  _product_coeff = new mfem::ScalarVectorProductCoefficient(_coef, vec_coef);
  return new mfem::DomainLFGradIntegrator(*_product_coeff);
}

#endif
