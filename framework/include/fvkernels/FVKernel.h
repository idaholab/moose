#pragma once

#include "MooseObject.h"
#include "TaggingInterface.h"
#include "TransientInterface.h"
#include "BlockRestrictable.h"

class SubProblem;

class FVKernel : public MooseObject,
                 public TaggingInterface,
                 public TransientInterface,
                 public BlockRestrictable
{
public:
  static InputParameters validParams();
  FVKernel(const InputParameters & params);

protected:
  SubProblem & _subproblem;
  THREAD_ID _tid;
  Assembly & _assembly;
};
