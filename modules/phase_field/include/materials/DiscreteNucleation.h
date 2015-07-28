#ifndef DISCRETENUCLEATION_H
#define DISCRETENUCLEATION_H

#include "DerivativeFunctionMaterialBase.h"
#include LIBMESH_INCLUDE_UNORDERED_MAP

// Forward declaration
class DiscreteNucleation;

template<>
InputParameters validParams<DiscreteNucleation>();

/**
 * Free energy penalty contribution to force the nucleation of subresolution particles
 */
class DiscreteNucleation : public DerivativeFunctionMaterialBase
{
public:
  DiscreteNucleation(const InputParameters & params);
  virtual ~DiscreteNucleation();

  virtual void computeProperties();

  /// We reset a counter in here to perform the nuclei insertion only once per timestep
  virtual void timestepSetup() { _insertion_counter = 0; }
  /// We increase the nuclei insertion counter here
  virtual void residualSetup() { _insertion_counter++; }

protected:
  struct NucleationEvent;
  typedef LIBMESH_BEST_UNORDERED_MAP<dof_id_type, std::vector<NucleationEvent *> > NucleationEventMap;

  unsigned int _nvar;

  /// map op_names indices to _args indices
  std::vector<unsigned int> _op_index;

  /// target concentration values
  const std::vector<Real> _op_values;

  const MaterialProperty<Real> & _probability;

  const Real _hold_time;
  const Real _penalty;

  unsigned int _insertion_counter;

  // map of currently active nucleation events for elements and QPs
  LIBMESH_BEST_UNORDERED_MAP<dof_id_type, std::vector<NucleationEvent *> > _nucleation_events;
};

#endif //DISCRETENUCLEATION_H
