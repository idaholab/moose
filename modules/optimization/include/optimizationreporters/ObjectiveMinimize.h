#pragma once

#include "OptimizationReporter.h"

class ObjectiveMinimize : public OptimizationReporter
{
public:
  static InputParameters validParams();
  ObjectiveMinimize(const InputParameters & parameters);
  virtual Real computeAndCheckObjective(bool solver_converged) override;

protected:
  virtual void updateParameters(const libMesh::PetscVector<Number> & x) override;

private:
  Real _bound_adjustment = 0.0;
};
