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

protected:
  unsigned int _id;                     ///< Unique ID of this component

  Simulation & _sim;                    ///< Simulation this component is part of
  R7Mesh & _mesh;                       ///< Global mesh this component works on
  FEProblem * & _problem;

  std::string _input_file_name;
  ComponentParser _parser;

  static unsigned int bc_ids;
};

#endif /* COMPONENT_H */
