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
//class ComponentPostProcessor;

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

  template<typename T>
  const T &
  getRParam(const std::string & param_name);

  template<typename T>
  void
  setRParam(const std::string & param_name, const T & value);

  /**
   * Get the names of MOOSE objects for given RAVEN name
   * @param rname The RAVEN name
   * @return the MOOSE object names
   */
  const std::vector<std::string> & getMooseObjectsByName(const std::string rname) { return _rname_map[rname]; }

  void connectObject(const std::string & rname, const std::string & mooseName);

  //LZou test
  //virtual void update();
  virtual void onResidual() {}
  virtual void onTimestepBegin() {}
  virtual void onTimestepEnd() {}

protected:
  unsigned int _id;                     ///< Unique ID of this component

  Simulation & _sim;                    ///< Simulation this component is part of
  R7Mesh & _mesh;                       ///< Global mesh this component works on
  Model::EModelType _model_type;        ///< Convenience variable that stores model type

  std::string _input_file_name;
  std::vector<unsigned int> _subdomains;     ///< List of subdomain IDs this components owns

  /// Mapping from a friendly name to MOOSE object name
  std::map<std::string, std::vector<std::string> > _rname_map;

  virtual unsigned int getNextSubdomainId();
  virtual unsigned int getNextBCId();

  static std::string genName(const std::string & prefix, unsigned int id, const std::string & suffix);
  static std::string genName(const std::string & prefix, const std::string & suffix);

  static std::vector<std::string> split(const std::string & rname);

private:
  // Do not want users to touch these, they _must_ use the API
  static unsigned int subdomain_ids;
  static unsigned int bc_ids;
};


#endif /* COMPONENT_H */
