//* This file is part of the MOOSE framework
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

namespace
{

class MeridionalCurlAthetaVectorCoefficient : public mfem::VectorCoefficient
{
public:
  MeridionalCurlAthetaVectorCoefficient(mfem::Coefficient & a_theta,
                                        mfem::VectorCoefficient & grad_a_theta)
    : mfem::VectorCoefficient(2), _a_theta(a_theta), _grad_a_theta(grad_a_theta)
  {
  }

  using mfem::VectorCoefficient::Eval;

  void Eval(mfem::Vector & V,
            mfem::ElementTransformation & T,
            const mfem::IntegrationPoint & ip) override
  {
    mfem::Vector grad;
    _grad_a_theta.Eval(grad, T, ip);

    mfem::Vector physical_point;
    T.Transform(ip, physical_point);

    const mfem::real_t r = physical_point[0];
    const mfem::real_t inv_r = 1.0 / r;

    const mfem::real_t a_theta = _a_theta.Eval(T, ip);

    // Meridional convention:
    //   x = r
    //   y = z
    const mfem::real_t dA_dr = grad[0];
    const mfem::real_t dA_dz = grad[1];

    V.SetSize(2);
    V[0] = -dA_dz;
    V[1] = dA_dr + a_theta * inv_r;
  }

private:
  mfem::Coefficient & _a_theta;
  mfem::VectorCoefficient & _grad_a_theta;
};

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
    _source_var_name(getParam<VariableName>("source")),
    _source_var(*getMFEMProblem().getGridFunction(_source_var_name))
{
}

void
MFEMMeridionalCurlAux::execute()
{
  mfem::GridFunctionCoefficient a_theta_coef(&_source_var);
  mfem::GradientGridFunctionCoefficient grad_a_theta_coef(&_source_var);

  MeridionalCurlAthetaVectorCoefficient curl_coef(a_theta_coef, grad_a_theta_coef);

  _result_var.ProjectCoefficient(curl_coef);
}

#endif
