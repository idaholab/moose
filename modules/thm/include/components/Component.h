#ifndef COMPONENT_H
#define COMPONENT_H

#include "Relap7App.h"
#include "R7Object.h"
#include "ActionWarehouse.h"
#include "FlowModel.h"
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
  struct ControlLogicNameEntry
  {
    ControlLogicNameEntry(const std::string & object_name, const std::string par_name) :
      _object_name(object_name),
      _par_name(par_name)
    {
    }

    /// MOOSE object name
    std::string _object_name;
    /// Parameter name
    std::string _par_name;
  };

  /**
   * Used to map friendly names to vector parameters, i.e. inlet:K_loss => K_loss[1], where K_loss is a vector parameter.
   */
  struct ControlLogicMapContainer
  {
    ControlLogicMapContainer(const std::string & controllable_par_name, unsigned int & position) :
      _controllable_par_name(controllable_par_name),
      _position(position)
    {
    }

    const std::string & getControllableParName() { return _controllable_par_name; }
    unsigned int & getControllableParPosition() { return _position; }

  protected:
    /// Variable name (i.e. K_loss)
    std::string _controllable_par_name;
    /// Position in the vector (i.e. 1)
    unsigned int _position;
  };

  Component(const std::string & name, InputParameters parameters);
  virtual ~Component();

  unsigned int id() { return _id; }


  /**
   * Initialize the component
   */
  virtual void init();

  /**
   * Builds mesh for this component and does bookkeeping
   */
  virtual void doBuildMesh();
  /**
   * Displace the reference mesh into 3D space
   */
  virtual void displaceMesh() = 0;

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
   * In case of an error, this method will throw a std::string exception with an error description.
   *
   * @param piece The name of the piece of the component you are interested in.
   */
  virtual std::string variableName(std::string piece) = 0;

  template<typename T>
  bool
  hasRParam(const std::string & param_name);

  template<typename T>
  const T &
  getRParam(const std::string & param_name);

  template<typename T>
  void
  setRParam(const std::string & param_name, const T & value);

  void aliasParam(const std::string & rname, const std::string & name, Component * comp = NULL);
  void aliasVectorParam(const std::string & rname, const std::string & name, unsigned int pos, Component * comp = NULL);
  /**
   * Connect with control logic
   */
  void connectObject(const std::string & rname, const std::string & mooseName, const std::string & name);
  /**
   * Connect with control logic
   */
  void connectObject(const std::string & rname, const std::string & mooseName, const std::string & name, const std::string & par_name);

  /**
   * This function creates a mapping between a control logic friendly name and a vector variable within a MOOSE object
   * @param rname  - control logic friendly name
   * @param mooseName - vector parameter name within an object
   * @param pos - position in the vector
   */
  void createVectorControllableParMapping(const std::string & rname, const std::string & mooseName, unsigned int pos);

  const std::map<std::string, std::map<std::string, std::vector<ControlLogicNameEntry> > > & getControllableParams() { return _rname_map; }

public:
  static std::string genName(const std::string & prefix, unsigned int id, const std::string & suffix);
  static std::string genName(const std::string & prefix, unsigned int i, unsigned int j, const std::string & suffix);
  static std::string genName(const std::string & prefix, const std::string & suffix);
  static std::string genName(const std::string & prefix, const std::string & middle, const std::string & suffix);

protected:
  /**
   * Build mesh for this component
   */
  virtual void buildMesh() = 0;

  /// Unique ID of this component
  unsigned int _id;
  /// Pointer to a parent component (used in composed components)
  Component * _parent;

  /// Simulation this component is part of
  Simulation & _sim;

  /// The Factory associated with the MooseApp
  Factory & _factory;

  /// Global mesh this component works on
  RELAP7Mesh & _mesh;
  /// Global physical mesh this component works on
  RELAP7Mesh * & _phys_mesh;

  /// List of subdomain IDs this components owns
  std::vector<unsigned int> _subdomains;

  /// Mapping from a friendly name to MOOSE object name
  std::map<std::string, std::map<std::string, std::vector<ControlLogicNameEntry> > > _rname_map;
  /// Mapping of friendly names
  std::map<std::string, ControlLogicMapContainer> _rvect_map;
  /// Map for aliasing component param names
  std::map<std::string, std::pair<Component *, std::string> > _param_alias_map;

  const Real & _zero;

  virtual unsigned int getNextSubdomainId();
  virtual unsigned int getNextBCId();

  /**
   * Split the control logic name into "section name" and "property name"
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
bool
Component::hasRParam(const std::string & param_name)
{
  std::vector<std::string> s = split(param_name);

  std::map<std::string, std::vector<ControlLogicNameEntry> > & rmap = _rname_map[s[0]];

  const std::vector<ControlLogicNameEntry> & entries = (rmap.find(s[1]) != rmap.end()) ? rmap[s[1]] : rmap[""];
  for (std::vector<ControlLogicNameEntry>::const_iterator it = entries.begin(); it != entries.end(); ++it)
  {
    const std::vector<MooseObject *> & objs = _sim.feproblem().getObjectsByName(it->_object_name, 0);
    if (objs.size() > 0)
    {
      std::string par_name = it->_par_name.empty() ? s[1] : it->_par_name;
      for (std::vector<MooseObject *>::const_iterator jt = objs.begin() ; jt != objs.end(); ++jt)
      {
        MooseObject * obj = *jt;
        if (obj->parameters().have_parameter<T>(par_name))
          return true;
      }
    }
  }

  // look for the parameter in this components' input parameters
  if (this->parameters().have_parameter<T>(param_name))
    return true;
  else
  {
    // not found, check in the alias map
    std::map<std::string, std::pair<Component *, std::string> >::iterator it = _param_alias_map.find(param_name);
    if (it != _param_alias_map.end())
    {
      Component * comp = it->second.first;
      std::string par_name = it->second.second;
      if (comp->parameters().have_parameter<T>(par_name))
        return true;
    }
  }

  // At this point the variable has not been found. Try to search in the vector parameter mapping.
  if (_rvect_map.find(param_name) == _rvect_map.end())
    return false;
  else
  {
    ControlLogicMapContainer name_cont = _rvect_map.find(param_name)->second;
    if (parameters().have_parameter<std::vector<T> >(name_cont.getControllableParName()))
      return true;
  }

  return false;
}

template<typename T>
const T &
Component::getRParam(const std::string & param_name)
{
  std::vector<std::string> s = split(param_name);

  std::map<std::string, std::vector<ControlLogicNameEntry> > & rmap = _rname_map[s[0]];

  const std::vector<ControlLogicNameEntry> & entries = (rmap.find(s[1]) != rmap.end()) ? rmap[s[1]] : rmap[""];
  for (std::vector<ControlLogicNameEntry>::const_iterator it = entries.begin(); it != entries.end(); ++it)
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

  // At this point the variable has not been found. Try to search in the vector parameter mapping.
  if (_rvect_map.find(param_name) == _rvect_map.end())
    mooseError(name() + ": parameter '" + param_name + "' was not found.");
  else
  {
    ControlLogicMapContainer name_cont = _rvect_map.find(param_name)->second;
    if (parameters().have_parameter<std::vector<T> >(name_cont.getControllableParName()))
      return parameters().get<std::vector<T> >(name_cont.getControllableParName())[name_cont.getControllableParPosition()];
  }
  mooseError(name() + ": parameter '" + param_name + "' was not found.");
}

template<typename T>
void
Component::setRParam(const std::string & param_name, const T & value)
{
  std::vector<std::string> s = split(param_name);

  std::map<std::string, std::vector<ControlLogicNameEntry> > & rmap = _rname_map[s[0]];
  const std::vector<ControlLogicNameEntry> & entries = (rmap.find(s[1]) != rmap.end()) ? rmap[s[1]] : rmap[""];
  for (std::vector<ControlLogicNameEntry>::const_iterator it = entries.begin(); it != entries.end(); ++it)
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

  // At this point the variable has not been found. Try to search in the vector parameter mapping
  if (_rvect_map.find(param_name) != _rvect_map.end())
  {
    ControlLogicMapContainer name_cont = _rvect_map.find(param_name)->second;
    if (parameters().have_parameter<std::vector<T> >(name_cont.getControllableParName()))
    {
      std::vector<T> tempp = parameters().get< std::vector<T> >(name_cont.getControllableParName());
      tempp[name_cont.getControllableParPosition()] = value;
      parameters().set<std::vector<T> >(name_cont.getControllableParName()) = tempp;
    }
  }
}

#endif /* COMPONENT_H */
