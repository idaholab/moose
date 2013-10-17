#include "ReportableDiffusion.h"

template<>
InputParameters validParams<ReportableDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<Real>("coef", 0.0, "The coefficient of diffusion");
  params.addParam<bool>("test_has_reportable", false, "Set to true to test the false case of hasReportableValue");
  params.addParam<bool>("report", true, "Output the flux calculation");
  return params;
}

ReportableDiffusion::ReportableDiffusion(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _coef(getParam<Real>("coef")),
    _calls(declareReportableValue("residual_calls", 0.0, getParam<bool>("report")))
{
  // Test the true case of hasReportableValue
  if (!hasReportableValue("residual_calls"))
    mooseError("Expected a reportable value for the name 'residual_calls', but it was not found");

  // Test the false case of hasReportableValue
  if (getParam<bool>("test_has_reportable"))
    if (!hasReportableValue("unknown"))
      mooseError("The name 'unknown' is not a reportable value");
}

Real
ReportableDiffusion::computeQpResidual()
{
  _calls += 1;
  return _coef*_grad_test[_i][_qp]*_grad_u[_qp];
}

Real
ReportableDiffusion::computeQpJacobian()
{
  return _coef*_grad_test[_i][_qp]*_grad_phi[_j][_qp];
}
