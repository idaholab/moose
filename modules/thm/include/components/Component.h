#ifndef COMPONENT_H
#define COMPONENT_H

#include "R7Object.h"
#include "R7Mesh.h"
#include "ComponentParser.h"
#include "ActionWarehouse.h"

class Simulation;
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

#endif /* COMPONENT_H */
