#pragma once

#include "FormFunction.h"

class ObjectiveGradientMinimize : public FormFunction
{
public:
  static InputParameters validParams();
  ObjectiveGradientMinimize(const InputParameters & parameters);

  virtual void computeGradient(libMesh::PetscVector<Number> & gradient) override;

private:
  /// vector of adjoint data
  const std::vector<Real> & _adjoint_data;
};
