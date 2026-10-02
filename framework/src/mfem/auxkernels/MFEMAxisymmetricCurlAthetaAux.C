//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMAxisymmetricCurlAthetaAux.h"

#include "MFEMCoordinateTransformations.h"
#include "MFEMProblem.h"
#include <cmath>

registerMooseObject("MooseApp", MFEMAxisymmetricCurlAthetaAux);

namespace
{

class AxisymmetricCurlAthetaVectorCoefficient : public mfem::VectorCoefficient
{
public:
  AxisymmetricCurlAthetaVectorCoefficient(mfem::Coefficient & a_theta,
                                          mfem::VectorCoefficient & grad_a_theta,
                                          mfem::Coefficient & inv_r)
    : mfem::VectorCoefficient(2), _a_theta(a_theta), _grad_a_theta(grad_a_theta), _inv_r(inv_r)
  {
  }

  using mfem::VectorCoefficient::Eval;

  void Eval(mfem::Vector & V,
            mfem::ElementTransformation & T,
            const mfem::IntegrationPoint & ip) override
  {
    mfem::Vector grad(2);
    _grad_a_theta.Eval(grad, T, ip);

    const mfem::real_t a_theta = _a_theta.Eval(T, ip);
    const mfem::real_t inv_r = _inv_r.Eval(T, ip);

    const mfem::real_t dA_dr = grad[0];
    const mfem::real_t dA_dz = grad[1];

    V.SetSize(2);
    V[0] = -dA_dz;
    V[1] = dA_dr + a_theta * inv_r;
  }

private:
  mfem::Coefficient & _a_theta;
  mfem::VectorCoefficient & _grad_a_theta;
  mfem::Coefficient & _inv_r;
};
}

InputParameters
MFEMAxisymmetricCurlAthetaAux::validParams()
{
  InputParameters params = MFEMAuxKernel::validParams();
  params.addClassDescription(
      "Calculates the axisymmetric curl associated with an azimuthal scalar variable A_theta "
      "and stores the result in a vector MFEM auxvariable.");

  MFEMExecutedObject::addRequiredDependencyParam<VariableName>(
      params, "source", "Scalar H1 MFEMVariable storing A_theta.");

  params.addRequiredParam<FunctionName>(
      "coordinate_function",
      "Name of the MFEMCoordinateTransformations function object. Must use coord_type = RZ.");

  return params;
}

MFEMAxisymmetricCurlAthetaAux::MFEMAxisymmetricCurlAthetaAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters),
    _source_var_name(getParam<VariableName>("source")),
    _source_var(*getMFEMProblem().getGridFunction(_source_var_name)),
    _coordinate_function(getParam<FunctionName>("coordinate_function")),
    _inv_r_coefficient(_coordinate_function + "_inv_r")
{
  const Function & func = getFunction("coordinate_function");

  const auto * coord_func = dynamic_cast<const MFEMCoordinateTransformations *>(&func);
  if (!coord_func)
    mooseError("Function '",
               _coordinate_function,
               "' supplied to MFEMAxisymmetricCurlAthetaAux is not of type "
               "MFEMCoordinateTransformations.");

  if (coord_func->coordType() != "RZ")
    mooseError("MFEMAxisymmetricCurlAthetaAux requires coordinate_function '",
               _coordinate_function,
               "' to use coord_type = RZ.");
}

void
MFEMAxisymmetricCurlAthetaAux::execute()
{
  mfem::GridFunctionCoefficient a_theta_coef(&_source_var);
  mfem::GradientGridFunctionCoefficient grad_a_theta_coef(&_source_var);

  auto & inv_r = getMFEMProblem().getCoefficients().getScalarCoefficient(_inv_r_coefficient);

  AxisymmetricCurlAthetaVectorCoefficient curl_coef(a_theta_coef, grad_a_theta_coef, inv_r);

  _result_var.ProjectCoefficient(curl_coef);
}
#endif
