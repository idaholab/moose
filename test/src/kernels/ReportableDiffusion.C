#include "ReportableDiffusion.h"

template<>
InputParameters validParams<ReportableDiffusion>()
{
  // Type of test to execute
  MooseEnum test("none, has_reportable, by_name", "none", "Select a test");
  InputParameters params = validParams<Kernel>();
  params.addParam<Real>("coef", 0.0, "The coefficient of diffusion");
  params.addParam<bool>("report", true, "Output the flux calculation");
  params.addParam<MooseEnum>("test", test, "Select the desired test");
  return params;
}

ReportableDiffusion::ReportableDiffusion(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _coef(getParam<Real>("coef")),
    _test(getParam<MooseEnum>("test")),
    _calls(_test == "by_name" ?
           declareReportableValueByName("residual_calls", 0.0, getParam<bool>("report")) :
           declareReportableValue("residual_calls", 0.0, getParam<bool>("report")))
{
  // Test the true case of hasReportableValue
  if (_test == "none" && !hasReportableValue("residual_calls"))
    mooseError("Expected a reportable value for the name 'residual_calls', but it was not found");

  // Test the false case of hasReportableValue
  if (_test == "has_reportable")
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
