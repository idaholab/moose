#ifndef COMPONENT_H
#define COMPONENT_H

#include "RELAP7.h"
#include "R7Object.h"
#include "R7Mesh.h"
#include "ActionWarehouse.h"
#include "Model.h"

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

  virtual const std::string & getType() = 0;

  // Location
  virtual Point getPosition() = 0;

  virtual RealVectorValue getDirection() = 0;

  virtual const std::vector<unsigned int> & getSubdomainIds() { return _subdomains; }

  /**
   * Coupling between components
   */
/*  virtual void RequestCoupling(std::string sender_name, std::string receiver_name, std::string ) 
	{ _sim.echoCouplingRequest(sender_name, receiver_name); }

  virtual void echoCouplingRequest(std::string sender_name) { }
*/   

  /**
   * Get the ids associated with the component.  These can either be subdomain ids or boundary ids depending
   * on what you are asking for.
   *
   * @param piece The name of the piece of the component you are interested in.
   */
  virtual std::vector<unsigned int> getIDs(std::string piece) = 0;

  /**
   * Returns the variable associated with that part of the component.
   *
   * @param piece The name of the piece of the component you are interested in.
   */
  virtual std::string variableName(std::string piece) = 0;

protected:
  unsigned int _id;                     ///< Unique ID of this component

  Simulation & _sim;                    ///< Simulation this component is part of
  R7Mesh & _mesh;                       ///< Global mesh this component works on
  Model::EModelType _model_type;        ///< Convenience variable that stores model type

  std::string _input_file_name;
  std::vector<unsigned int> _subdomains;     ///< List of subdomain IDs this components owns

  virtual unsigned int getNextSubdomainId();
  virtual unsigned int getNextBCId();

  static std::string genName(const std::string & prefix, unsigned int id, const std::string & suffix);
  static std::string genName(const std::string & prefix, const std::string & suffix);

private:
  // Do not want users to touch these, they _must_ use the API
  static unsigned int subdomain_ids;
  static unsigned int bc_ids;
};

#endif /* COMPONENT_H */
