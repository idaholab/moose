#ifndef COMPONENT_H
#define COMPONENT_H

#include "RELAP7.h"
#include "R7Object.h"
#include "ActionWarehouse.h"
#include "Model.h"
#include "Simulation.h"

class Component;
class FEProblem;
class RavenMapContainer;

template<>
InputParameters validParams<Component>();

/*
 * Class used by Component class to map vector parameters through friendly names
 * i.e. friendly name = inlet:K_loss; variableName = K_loss, position = 1.
 * Since this class is only privately used by the class Component,
 * it's been added in this file
 */
class RavenMapContainer
{
public:
   RavenMapContainer();
   RavenMapContainer(const std::string & controllableParName, unsigned int & position);
   virtual ~ RavenMapContainer();
   const std::string & getControllableParName();
   unsigned int & getControllableParPosition();
protected:
   /// Variable name (i.e. K_loss)
   std::string _controllableParName;
   /// Position in the vector (i.e. 1)
   unsigned int _position;
};


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

  /**
   * This function creates a mapping between a RAVEN friendly name and a vector variable within a MOOSE object
   * @param rname  - RAVEN friendly name
   * @param mooseName - vector parameter name within an object
   * @param pos - position in the vector
   */
  void createVectorControllableParMapping(const std::string & rname, const std::string & mooseName, unsigned int pos);

  virtual void onResidual() {}
  virtual void onTimestepBegin() {}
  virtual void onTimestepEnd() {}

public:
  static std::string genName(const std::string & prefix, unsigned int id, const std::string & suffix);
  static std::string genName(const std::string & prefix, const std::string & suffix);
  static std::string genName(const std::string & prefix, const std::string & middle, const std::string & suffix);

protected:
  /// Unique ID of this component
  unsigned int _id;
  /// Pointer to a parent component (used in composed components)
  Component * _parent;

  /// Simulation this component is part of
  Simulation & _sim;
  /// Global mesh this component works on
  MooseMesh & _mesh;
  /// Global physical mesh this component works on
  MooseMesh * & _phys_mesh;
  /// Convenience variable that stores model type
  Model::EModelType _model_type;

  std::string _input_file_name;
  /// List of subdomain IDs this components owns
  std::vector<unsigned int> _subdomains;

  /// Mapping from a friendly name to MOOSE object name
  std::map<std::string, std::vector<std::string> > _rname_map;

  /// Mapping from a friendly name to a vector variable within a MOOSE object
  std::map<std::string, RavenMapContainer> _rvect_map;

  virtual unsigned int getNextSubdomainId();
  virtual unsigned int getNextBCId();

  /**
   * Checks the consistency of model and EOS objects
   */
  void checkEOSConsistency();

  /**
   * Split the "RAVEN" name into "section name" and "property name"
   * @param rname
   * @return
   */
  static std::vector<std::string> split(const std::string & rname);

private:
  // Do not want users to touch these, they _must_ use the API
  static unsigned int subdomain_ids;
  static unsigned int bc_ids;
};



template<typename T>
const T &
Component::getRParam(const std::string & param_name)
{
  std::vector<std::string> s = split(param_name);

  const std::vector<std::string> & names = getMooseObjectsByName(s[0]);
  for (std::vector<std::string>::const_iterator it = names.begin(); it != names.end(); ++it)
  {
    std::string nm = *it;
    const std::vector<MooseObject *> & objs = _sim.feproblem().getObjectsByName(nm, 0);
    for (std::vector<MooseObject *>::const_iterator it = objs.begin() ; it != objs.end(); ++it)
    {
      MooseObject * obj = *it;
      if (obj->parameters().have_parameter<T>(s[1]))
        return obj->parameters().get<T>(s[1]);
    }
  }

  // Specialization for RAVEN. At this point the variable has not been found.
  // Try to search into the vector parameter mapping.
  if (_rvect_map.find(param_name) == _rvect_map.end())
    mooseError("Parameter '" + param_name + "' was not found in component '" + name() + "'.");
  else
  {
    RavenMapContainer name_cont = _rvect_map.find(param_name)->second;
    if (parameters().have_parameter<std::vector<T> >(name_cont.getControllableParName()))
      return parameters().get<std::vector<T> >(name_cont.getControllableParName())[name_cont.getControllableParPosition()];
  }
  mooseError("Parameter '" + param_name + "' was not found in component '" + name() + "'.");
}

template<typename T>
void
Component::setRParam(const std::string & param_name, const T & value)
{
  std::vector<std::string> s = split(param_name);
  bool found = false;

  const std::vector<std::string> & names = getMooseObjectsByName(s[0]);
  for (std::vector<std::string>::const_iterator it = names.begin(); it != names.end(); ++it)
  {
    std::string nm = *it;
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    {
      const std::vector<MooseObject *> & objs = _sim.feproblem().getObjectsByName(nm, tid);

      for (std::vector<MooseObject *>::const_iterator it = objs.begin(); it != objs.end(); ++it)
      {
        MooseObject * obj = *it;
        if (obj->parameters().have_parameter<T>(s[1]))
        {
          obj->parameters().set<T>(s[1]) = value;
          found = true;
        }
      }
    }
  }

  // Specialization for RAVEN. At this point the variable has not been found.
  // Try to search into the vector parameter mapping
  if (!found)
  {
    if (_rvect_map.find(param_name) != _rvect_map.end())
    {
      RavenMapContainer name_cont = _rvect_map.find(param_name)->second;
      if (parameters().have_parameter<std::vector<T> >(name_cont.getControllableParName()))
      {
        std::vector<T> tempp = parameters().get< std::vector<T> >(name_cont.getControllableParName());
        tempp[name_cont.getControllableParPosition()] = value;
        parameters().set<std::vector<T> >(name_cont.getControllableParName()) = tempp;
      }
    }
  }
}

#endif /* COMPONENT_H */
