#pragma once

#include "FlowJunction.h"

/**
 * Junction that adds periodic BC for all variables
 */
class PeriodicJunction : public FlowJunction
{
public:
  PeriodicJunction(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  virtual void setupMesh() override;
  virtual void check() const override;

  /// Adds periodic BC for a list of variables
  void addPeriodicBC(const std::vector<VariableName> variables) const;

  /// Vector which when added to primary boundary coordinates gives secondary boundary coordinates
  RealVectorValue _translation_vector;

public:
  static InputParameters validParams();
};
