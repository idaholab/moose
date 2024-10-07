#include "MFEMDataCollection.h"

InputParameters
MFEMDataCollection::validParams()
{
  InputParameters params = FileOutput::validParams();
  params.addClassDescription("Output for controlling MFEMDataCollection inherited data.");
  return params;
}

MFEMDataCollection::MFEMDataCollection(const InputParameters & parameters)
  : FileOutput(parameters),
    _problem_data(static_cast<MFEMProblem *>(_problem_ptr)->getProblemData())
{
}
