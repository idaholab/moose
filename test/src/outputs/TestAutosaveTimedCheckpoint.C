#include "TestAutosaveTimedCheckpoint.h"

registerMooseObject("MooseTestApp", TestAutosaveTimedCheckpoint);

InputParameters
TestAutosaveTimedCheckpoint::validParams()
{
  InputParameters params = Checkpoint::validParams();
  params.addParam<bool>(
      "test_system", false, "Test the system autosave rather than a regular checkpoint");
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
  if (getParam<bool>("test_system"))
    setAutosaveFlag(AutosaveType::SYSTEM_AUTOSAVE);
  Checkpoint::initialSetup();
}
