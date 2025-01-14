#include "MFEMExecutioner.h"
#include "MFEMProblem.h"

InputParameters
MFEMExecutioner::validParams()
{
  InputParameters params = Executioner::validParams();
  params.addClassDescription("Executioner for MFEM problems.");
  params.addParam<std::string>("device", "cpu", "Run app on the chosen device.");
  MooseEnum assembly_levels("legacy full element partial none", "legacy", true);
  params.addParam<MooseEnum>(
      "assembly_level",
      assembly_levels,
      "Matrix assembly level. Options: legacy, full, element, partial, none.");

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
