#pragma once

#include "FormFunction.h"

class ObjectiveMinimize : public FormFunction
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
