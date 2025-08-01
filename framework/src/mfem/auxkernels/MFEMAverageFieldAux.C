#ifdef MFEM_ENABLED

#include "MFEMAverageFieldAux.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMAverageFieldAux);

InputParameters
MFEMAverageFieldAux::validParams()
{
  InputParameters params = MFEMAuxKernel::validParams();
  params.addClassDescription(
      "Time-weighted running average of a MFEMVariable, projected into an AuxVariable.");
  params.addRequiredParam<VariableName>("source", "Name of the MFEMVariable to average from.");
  params.addRequiredParam<mfem::real_t>("time_step", "Simulation time-step size dt.");
  params.addParam<mfem::real_t>("timestep_skip", 0.0, "Time to skip before beginning the average.");

  return params;
}

MFEMAverageFieldAux::MFEMAverageFieldAux(const InputParameters & parameters)
  : MFEMAuxKernel(parameters),
    _source_var_name(getParam<VariableName>("source")),
    _source_var(*getMFEMProblem().getProblemData().gridfunctions.Get(_source_var_name)),
    _dt(getParam<mfem::real_t>("time_step")),
    _skip(getParam<mfem::real_t>("timestep_skip"))
//)
{
}

void
MFEMAverageFieldAux::execute()
{
  // project AvgCoef; it will read the old average from _result_var
  // and the current field from _source_var, compute w = dt/(t-skip), and
  // return (1-w)*old + w*new for each integration point.
  mfem::real_t t = getMFEMProblem().time();

  AvgCoef coef(_result_var, _source_var, t, _dt, _skip);

  _result_var.ProjectCoefficient(coef);
}

MFEMAverageFieldAux::AvgCoef::~AvgCoef() {}

#endif
