#include "MFEMExecutioner.h"
#include "MFEMProblem.h"

InputParameters
MFEMExecutioner::validParams()
{
  return Executioner::validParams();
}

MFEMExecutioner::MFEMExecutioner(const InputParameters & parameters)
  : Executioner(parameters),
    _mfem_problem(dynamic_cast<MFEMProblem &>(feProblem())),
    _problem_data(_mfem_problem.getProblemData())
{
}
