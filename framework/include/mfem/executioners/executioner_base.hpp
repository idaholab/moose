#pragma once
#include "inputs.hpp"

namespace hephaestus
{

class Executioner
{
public:
  Executioner() = default;
  explicit Executioner(const hephaestus::InputParameters & params) {}

  virtual ~Executioner() = default;

  // Solve the current system of equations
  virtual void Solve() const = 0;

  // Execute solution strategy including any timestepping
  virtual void Execute() const = 0;
};

} // namespace hephaestus
