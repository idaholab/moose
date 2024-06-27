#pragma once
#include "coefficients.hpp"
#include "gridfunctions.hpp"
#include "hephaestus_solvers.hpp"
#include "inputs.hpp"
#include "mfem.hpp"

// Specify classes that perform auxiliary calculations on GridFunctions or
// Coefficients.
namespace hephaestus
{

class AuxSolver;

class AuxSolver
{
public:
  AuxSolver() = default;

  // NB: require virtual destructor to avoid leaks.
  virtual ~AuxSolver() = default;

  virtual void Init(const hephaestus::GridFunctions & gridfunctions,
                    hephaestus::Coefficients & coefficients) = 0;

  virtual void Solve(double t = 0.0) = 0;

  // Set priority. Lower values are evaluated first.
  void SetPriority(const int priority) { _priority = priority; };

  [[nodiscard]] inline int Priority() const { return _priority; }

  static bool PriorityComparator(
      const std::pair<std::shared_ptr<hephaestus::AuxSolver>, std::string> & first_comp,
      const std::pair<std::shared_ptr<hephaestus::AuxSolver>, std::string> & second_comp)
  {
    return (first_comp.first->Priority() < second_comp.first->Priority());
  }

private:
  int _priority{0};
};

} // namespace hephaestus
