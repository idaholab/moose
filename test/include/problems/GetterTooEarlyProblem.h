//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov

#pragma once

#include "FEProblem.h"

class Function;
class UserObject;

class GetterTooEarlyProblem : public FEProblem
{
public:
  static InputParameters validParams();

  GetterTooEarlyProblem(const InputParameters & params);

protected:
  const Function * _function;
  const UserObject * _user_object;
};
