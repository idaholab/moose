#pragma once

#include "Checkpoint.h"

class TestAutosaveTimedCheckpoint : public Checkpoint
{
public:
  static InputParameters validParams();

  TestAutosaveTimedCheckpoint(const InputParameters & parameters);
  virtual ~TestAutosaveTimedCheckpoint();

  virtual void initialSetup() override;
};
