#ifndef AVERAGENODALVARIABLEVALUE_H_
#define AVERAGENODALVARIABLEVALUE_H_

#include "GeneralPostprocessor.h"

//Forward Declarations
class AverageNodalVariableValue;

namespace Moose {
  class Mesh;
}

template<>
InputParameters validParams<AverageNodalVariableValue>();

class AverageNodalVariableValue : public GeneralPostprocessor
{
public:
  AverageNodalVariableValue(const std::string & name, InputParameters parameters);
  
  virtual void initialize() {}
  virtual void execute() {}

  /**
   * This will return the degrees of freedom in the system.
   */
  virtual Real getValue();

protected:
  Moose::Mesh & _mesh;
  std::string _var_name;
  unsigned int _nodesetid;

  std::vector<unsigned int> _node_ids;
};

#endif //AVERAGENODALVARIABLEVALUE_H_
