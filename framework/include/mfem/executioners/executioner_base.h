#pragma once
#include "inputs.h"

namespace platypus
{

class Executioner
{
public:
  Executioner() = default;
  explicit Executioner(const platypus::InputParameters & params) {}

  virtual ~Executioner() = default;

  // Solve the current system of equations
  virtual void Solve() const = 0;

  // Execute solution strategy including any timestepping
  virtual void Execute() const = 0;
};

} // namespace platypus
