#ifndef COMPONENT_H
#define COMPONENT_H

#include "R7Object.h"
#include "R7Mesh.h"
#include "R7_Moose.h"
#include "ComponentParser.h"
#include "ActionWarehouse.h"
#include "Simulation.h"


class Component;
class FEProblem;

template<>
InputParameters validParams<Component>();

/**
 * Base class for R7 components
 */
class Component : public R7Object
{
public:
  Component(const std::string & name, InputParameters parameters);
  virtual ~Component();

  unsigned int id() { return _id; }

  /**
   * Get parameter from simulation (global one)
   * @param name The name on the parameter
   * @return The value of the parameter
   */
  template<typename T>
  const T & getSimParam(const std::string & name);

  /**
   * Initialize the component
   */
  virtual void init();

  /**
   * Build mesh for this component
   */
  virtual void buildMesh() = 0;

  virtual void addVariables() = 0;

  virtual void addMooseObjects() { }

  virtual void parseInput();

  // Location
  virtual Point getPosition() = 0;

  virtual RealVectorValue getDirection() = 0;

  virtual const std::vector<unsigned int> & getSubdomainIds() { return _subdomain_ids; }

protected:
  unsigned int _id;                     ///< Unique ID of this component

  Simulation & _sim;                    ///< Simulation this component is part of
  R7Mesh & _mesh;                       ///< Global mesh this component works on
  FEProblem * & _problem;

  std::string _input_file_name;
  ComponentParser _parser;
  std::vector<unsigned int> _subdomain_ids;     ///< List of subdomain IDs this components owns

  virtual unsigned int getNextSubdomainId();

  static unsigned int bc_ids;
  static std::string genName(const std::string & prefix, unsigned int id, const std::string & suffix);

private:
  // Do not want users to touch this, they _must_ use the API
  static unsigned int subdomain_ids;
};


template<typename T>
const T & Component::getSimParam(const std::string & name)
{
  return _sim.params().get<T>(name);
}

#endif /* COMPONENT_H */
