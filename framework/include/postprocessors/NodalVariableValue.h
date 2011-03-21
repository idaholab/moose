#ifndef NODALVARIABLEVALUE_H_
#define NODALVARIABLEVALUE_H_

#include "GeneralPostprocessor.h"
// libMesh
#include "node.h"

namespace Moose {
  class Mesh;
}

//Forward Declarations
class NodalVariableValue;

template<>
InputParameters validParams<NodalVariableValue>();

class NodalVariableValue : public GeneralPostprocessor
{
public:
  NodalVariableValue(const std::string & name, InputParameters parameters);
  
  virtual void initialize() {}
  virtual void execute() {}

  /**
   * This will return the degrees of freedom in the system.
   */
  virtual Real getValue();

protected:
  Moose::Mesh & _mesh;
  std::string _var_name;
  Node & _node;
};

#endif //NODALVARIABLEVALUE_H_
