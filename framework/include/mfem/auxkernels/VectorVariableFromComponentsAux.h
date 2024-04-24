#pragma once

#include "WritableVectorAuxKernel.h"

/**
 * Construct a vector variable from 3 standard variables.
 */
class VectorVariableFromComponentsAux : public WritableVectorAuxKernel
{
public:
  static InputParameters validParams();

  VectorVariableFromComponentsAux(const InputParameters & parameters);

protected:
  virtual void compute() override;

  MooseVariable & _component_x;
  MooseVariable & _component_y;
  MooseVariable & _component_z;

  const Order _vector_order;
  const FEFamily _vector_family;

private:
  void checkVectorVariable() const;
  void checkVectorComponents() const;
  void checkVectorComponent(const MooseVariable & component_variable) const;
};
