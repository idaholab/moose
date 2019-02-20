#ifndef PERIODICJUNCTION_H
#define PERIODICJUNCTION_H

#include "FlowJunction.h"

class PeriodicJunction;

template <>
InputParameters validParams<PeriodicJunction>();

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
};

#endif /* PERIODICJUNCTION_H */
