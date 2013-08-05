#ifndef COMPONENT_H
#define COMPONENT_H

#include "Relap7App.h"
#include "R7Object.h"
#include "ActionWarehouse.h"
#include "FlowModel.h"
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
  struct RavenNameEntry
  {
    RavenNameEntry(const std::string & object_name, const std::string par_name) :
      _object_name(object_name),
      _par_name(par_name)
    {
    }

    /// MOOSE object name
    std::string _object_name;
    /// Parameter name
    std::string _par_name;
  };

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

  void aliasParam(const std::string & rname, const std::string & name, Component * comp = NULL);
  void aliasVectorParam(const std::string & rname, const std::string & name, unsigned int pos, Component * comp = NULL);
  void connectObject(const std::string & rname, const std::string & mooseName, const std::string & name);
  void connectObject(const std::string & rname, const std::string & mooseName, const std::string & name, const std::string & par_name);

  /**
   * This function creates a mapping between a RAVEN friendly name and a vector variable within a MOOSE object
   * @param rname  - RAVEN friendly name
   * @param mooseName - vector parameter name within an object
   * @param pos - position in the vector
   */
  void createVectorControllableParMapping(const std::string & rname, const std::string & mooseName, unsigned int pos);

  const std::map<std::string, std::map<std::string, std::vector<RavenNameEntry> > > & getControllableParams() { return _rname_map; }

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

  /// The Factory associated with the MooseApp
  Factory & _factory;

  /// Global mesh this component works on
  MooseMesh & _mesh;
  /// Global physical mesh this component works on
  MooseMesh * & _phys_mesh;

  std::string _input_file_name;
  /// List of subdomain IDs this components owns
  std::vector<unsigned int> _subdomains;

  /// Mapping from a friendly name to MOOSE object name
  std::map<std::string, std::map<std::string, std::vector<RavenNameEntry> > > _rname_map;
  /// Mapping of friendly names
  std::map<std::string, RavenMapContainer> _rvect_map;
  /// Map for aliasing component param names
  std::map<std::string, std::pair<Component *, std::string> > _param_alias_map;

  const Real & _zero;

  virtual unsigned int getNextSubdomainId();
  virtual unsigned int getNextBCId();

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

  std::map<std::string, std::vector<RavenNameEntry> > & rmap = _rname_map[s[0]];

  const std::vector<RavenNameEntry> & entries = (rmap.find(s[1]) != rmap.end()) ? rmap[s[1]] : rmap[""];
  for (std::vector<RavenNameEntry>::const_iterator it = entries.begin(); it != entries.end(); ++it)
  {
    const std::vector<MooseObject *> & objs = _sim.feproblem().getObjectsByName(it->_object_name, 0);
    if (objs.size() > 0)
    {
      std::string par_name = it->_par_name.empty() ? s[1] : it->_par_name;
      for (std::vector<MooseObject *>::const_iterator jt = objs.begin() ; jt != objs.end(); ++jt)
      {
        MooseObject * obj = *jt;
        if (obj->parameters().have_parameter<T>(par_name))
          return obj->parameters().get<T>(par_name);
      }
    }
  }

  // look for the parameter in this components' input parameters
  if (this->parameters().have_parameter<T>(param_name))
    return this->parameters().get<T>(param_name);
  else
  {
    // not found, check in the alias map
    std::map<std::string, std::pair<Component *, std::string> >::iterator it = _param_alias_map.find(param_name);
    if (it != _param_alias_map.end())
    {
      Component * comp = it->second.first;
      std::string par_name = it->second.second;
      if (comp->parameters().have_parameter<T>(par_name))
        return comp->parameters().get<T>(par_name);
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

  std::map<std::string, std::vector<RavenNameEntry> > & rmap = _rname_map[s[0]];
  const std::vector<RavenNameEntry> & entries = (rmap.find(s[1]) != rmap.end()) ? rmap[s[1]] : rmap[""];
  for (std::vector<RavenNameEntry>::const_iterator it = entries.begin(); it != entries.end(); ++it)
  {
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    {
      const std::vector<MooseObject *> & objs = _sim.feproblem().getObjectsByName(it->_object_name, tid);
      std::string par_name = it->_par_name.empty() ? s[1] : it->_par_name;
      for (std::vector<MooseObject *>::const_iterator jt = objs.begin(); jt != objs.end(); ++jt)
      {
        MooseObject * obj = *jt;
        if (obj->parameters().have_parameter<T>(par_name))
        {
          obj->parameters().set<T>(par_name) = value;
        }
      }
    }
  }

  // look for the parameter in this components' input parameters
  if (this->parameters().have_parameter<T>(param_name))
  {
    this->parameters().set<T>(param_name) = value;
  }
  else
  {
    // not found, check in the alias map
    std::map<std::string, std::pair<Component *, std::string> >::iterator it = _param_alias_map.find(param_name);
    if (it != _param_alias_map.end())
    {
      Component * comp = it->second.first;
      std::string par_name = it->second.second;
      if (comp->parameters().have_parameter<T>(par_name))
        comp->parameters().set<T>(par_name) = value;
    }
  }

  // Specialization for RAVEN. At this point the variable has not been found.
  // Try to search into the vector parameter mapping
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

#endif /* COMPONENT_H */
