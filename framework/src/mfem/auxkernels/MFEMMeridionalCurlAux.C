///* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMeridionalCurlAux.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMMeridionalCurlAux);

namespace Moose::MFEM
{
MeridionalCurlVectorCoefficient::MeridionalCurlVectorCoefficient(
    mfem::Coefficient & a_theta, mfem::VectorCoefficient & grad_a_theta)
  : mfem::VectorCoefficient(2), _a_theta(a_theta), _grad_a_theta(grad_a_theta)
{
}

void
MeridionalCurlVectorCoefficient::Eval(mfem::Vector & V,
                                      mfem::ElementTransformation & T,
                                      const mfem::IntegrationPoint & ip)
{
  mfem::Vector A_grad;
  _grad_a_theta.Eval(A_grad, T, ip);

  mfem::Vector physical_point;
  T.Transform(ip, physical_point);

  const mfem::real_t r = physical_point[0];
  const mfem::real_t A = _a_theta.Eval(T, ip);

  const mfem::real_t dA_dr = A_grad[0];
  const mfem::real_t dA_dz = A_grad[1];

  V.SetSize(2);
  V[0] = -dA_dz;
  V[1] = dA_dr + A / r;
}
}

InputParameters
MFEMMeridionalCurlAux::validParams()
{
  InputParameters params = MFEMAuxKernel::validParams();

  params.addClassDescription(
      "Calculates the meridional curl associated with an azimuthal scalar variable A_theta "
      "on a 2D meridional mesh with x = r and y = z, and stores the result in a vector "
      "MFEM auxvariable.");

  MFEMExecutedObject::addRequiredDependencyParam<VariableName>(
      params, "source", "Scalar H1 MFEMVariable storing A_theta.");

  return params;
}

MFEMMeridionalCurlAux::MFEMMeridionalCurlAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters),
    _curl_coef(getScalarCoefficientByName(getParam<VariableName>("source")),
               getVectorCoefficientByName(getParam<VariableName>("source") + "_grad"))
{
}

void
MFEMMeridionalCurlAux::execute()
{
  _result_var.ProjectCoefficient(_curl_coef);
}

#endif
