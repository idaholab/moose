#include "TestAutosaveTimedCheckpoint.h"

registerMooseObject("MooseTestApp", TestAutosaveTimedCheckpoint);

InputParameters
TestAutosaveTimedCheckpoint::validParams()
{
  InputParameters params = Checkpoint::validParams();
  // params.setParameters("is_autosave", AutosaveType::SYSTEM_AUTOSAVE);
  // params.setParameters("file_base", std::string("autosave"));
  return params;
}

TestAutosaveTimedCheckpoint::TestAutosaveTimedCheckpoint(const InputParameters & parameters)
  : Checkpoint(parameters)
{
}

TestAutosaveTimedCheckpoint::~TestAutosaveTimedCheckpoint() {}

void
TestAutosaveTimedCheckpoint::initialSetup()
{
  _is_autosave = AutosaveType::SYSTEM_AUTOSAVE;
  Checkpoint::initialSetup();
}
