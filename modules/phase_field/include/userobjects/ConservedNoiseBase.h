/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CONSERVEDNOISEBASE_H
#define CONSERVEDNOISEBASE_H

#include "ConservedNoiseInterface.h"

#include <unordered_map>

// Forward Declarations
class ConservedNoiseBase;

template <>
InputParameters validParams<ConservedNoiseBase>();

/**
  * This Userobject is the base class of Userobjects that generate one
  * random number per timestep and quadrature point in a way that the integral
  * over all random numbers is zero. This can be used for a concentration fluctuation
  * kernel such as ConservedLangevinNoise, that keeps the total concenration constant.
  *
  * \see ConservedUniformNoise
  * \see ConservedNormalNoise
  */
class ConservedNoiseBase : public ConservedNoiseInterface
{
public:
  ConservedNoiseBase(const InputParameters & parameters);

  virtual ~ConservedNoiseBase() {}

  virtual void initialize();
  virtual void execute();
  virtual void threadJoin(const UserObject & y);
  virtual void finalize();

  Real getQpValue(dof_id_type element_id, unsigned int qp) const;

protected:
  std::unordered_map<dof_id_type, std::vector<Real>> _random_data;
};

#endif // CONSERVEDNOISEBASE_H
