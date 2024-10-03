#include "MFEMExecutioner.h"
#include "MFEMProblem.h"

InputParameters
MFEMExecutioner::validParams()
{
  InputParameters params = Executioner::validParams();
  params.addParam<std::string>("device", "cpu", "Run app on the chosen device.");
  return params;
}

MFEMExecutioner::MFEMExecutioner(const InputParameters & parameters)
  : Executioner(parameters),
    _mfem_problem(dynamic_cast<MFEMProblem &>(feProblem())),
    _problem_data(_mfem_problem.getProblemData())
{
  setDevice();
}

void
MFEMExecutioner::setDevice()
{
  _device.Configure(getParam<std::string>("device"));
  _device.Print(std::cout);
}
