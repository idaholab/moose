#include "ReportableDiffusion.h"

template<>
InputParameters validParams<ReportableDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<Real>("coef", 0.0, "The coefficient of diffusion");
  params.addParam<bool>("test_has_reportable", false, "Set to true to test the false case of hasReportableValue");
  params.addParam<bool>("output_coef", true, "Output the coefficient");
  return params;
}

ReportableDiffusion::ReportableDiffusion(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _coef(declareReportableValue("coef", getParam<Real>("coef"), getParam<bool>("output_coef")))
{
  // Test the true case of hasReportableValue
  if (!hasReportableValue("coef"))
    mooseError("Expected a reportable value for the name 'coef', but it was not found");

  // Test the false case of hasReportableValue
  if (getParam<bool>("test_has_reportable"))
    if (!hasReportableValue("unknown"))
      mooseError("The name 'unknown' is not a reportable value");
}

Real
ReportableDiffusion::computeQpResidual()
{
  Real k = getReportableValue("coef");
  return k*_grad_test[_i][_qp]*_grad_u[_qp];
}

Real
ReportableDiffusion::computeQpJacobian()
{
  Real k = getReportableValue("coef");
  return k*_grad_test[_i][_qp]*_grad_phi[_j][_qp];
}
