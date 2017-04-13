/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CONSERVEDMASKEDNOISEBASE_H
#define CONSERVEDMASKEDNOISEBASE_H

#include "ConservedNoiseInterface.h"

#include <unordered_map>

// Forward Declarations
class ConservedMaskedNoiseBase;

template <>
InputParameters validParams<ConservedMaskedNoiseBase>();

/**
 * This Userobject is the base class of Userobjects that generate one
 * random number per timestep and quadrature point in a way that the integral
 * over all random numbers is zero. It behaves as ConservedNoiseBase but allows
 * the user to specify a multiplicator in the form of a MaterialProperty that is
 * multiplied on each random number, effectively masking the random number field.
 *
 * \see ConservedUniformNoise
 * \see ConservedNormalNoise
 */
class ConservedMaskedNoiseBase : public ConservedNoiseInterface
{
public:
  ConservedMaskedNoiseBase(const InputParameters & parameters);

  virtual ~ConservedMaskedNoiseBase() {}

  virtual void initialize();
  virtual void execute();
  virtual void threadJoin(const UserObject & y);
  virtual void finalize();

  Real getQpValue(dof_id_type element_id, unsigned int qp) const;

protected:
  std::unordered_map<dof_id_type, std::vector<std::pair<Real, Real>>> _random_data;

  const MaterialProperty<Real> & _mask;
};

#endif // CONSERVEDMASKEDNOISEBASE_H
