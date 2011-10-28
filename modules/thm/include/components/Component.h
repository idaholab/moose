#ifndef COMPONENT_H
#define COMPONENT_H

#include "R7Object.h"
#include "MooseMesh.h"

class Component;

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


  template<typename T>
  const T & getParam(const std::string & name);

protected:
  unsigned int _id;                     ///< Unique ID of this component

  MooseMesh & _mesh;                    ///< Global mesh this component works on
};


template<typename T>
const T & Component::getParam(const std::string & name)
{
  return _pars.get<T>(name);
}

#endif /* COMPONENT_H */
