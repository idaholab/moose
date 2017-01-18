/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef COARSENINGINTEGRALTRACKER_H
#define COARSENINGINTEGRALTRACKER_H

#include "ElementUserObject.h"

class CoarseningIntegralTracker;

template<>
InputParameters validParams<CoarseningIntegralTracker>();

/**
 * Track changes in the element integral of a variable due to mesh coarsening.
 */
class CoarseningIntegralTracker : public ElementUserObject
{
public:
  CoarseningIntegralTracker(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override {};

  virtual void meshChanged() override;

  // joined the source term maps
  virtual void threadJoin(const UserObject & y) override;

  // obtain the corrective source term value
  Real sourceValue(const Elem *);

protected:
  /// simulation mesh
  MooseMesh & _mesh;

  /// variable to correct
  const VariableValue & _v;

  /// pre-adaptivity element integrals
  std::map<const Elem *, Real> _pre_adaptivity_integral;

  /// corrective source term
  std::map<const Elem *, Real> _corrective_source;

  /// flag to indicate if a pre-adaptivity run was performed (decided by dependency resolution)
  bool _pre_adaptivity_ran;
};

#endif //COARSENINGINTEGRALTRACKER_H
