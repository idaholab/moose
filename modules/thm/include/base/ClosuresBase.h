#ifndef CLOSURESBASE_H
#define CLOSURESBASE_H

#include "MooseObject.h"
#include "LoggingInterface.h"

class ClosuresBase;
class Pipe;
class Simulation;
class Factory;

template <>
InputParameters validParams<ClosuresBase>();

/**
 * Base class for closures implementations
 *
 * The responsibilities of the closures objects depends on the flow model that
 * uses them. Examples of responsibilities will be to provide material properties
 * for friction factors and heat transfer coefficients.
 */
class ClosuresBase : public MooseObject, public LoggingInterface
{
public:
  ClosuresBase(const InputParameters & params);

  /**
   * Checks for errors
   *
   * @param[in] flow_channel   Flow channel component
   */
  virtual void check(const Pipe & flow_channel) const = 0;

  /**
   * Adds MOOSE objects
   *
   * @param[in] flow_channel   Flow channel component
   */
  virtual void addMooseObjects(const Pipe & flow_channel) = 0;

protected:
  /// Simulation
  Simulation & _sim;

  /// Factory associated with the MooseApp
  Factory & _factory;
};

#endif /* CLOSURESBASE_H */
