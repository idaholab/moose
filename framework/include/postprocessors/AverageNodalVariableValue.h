#ifndef AVERAGENODALVARIABLEVALUE_H
#define AVERAGENODALVARIABLEVALUE_H

#include "GeneralPostprocessor.h"

//Forward Declarations
class AverageNodalVariableValue;
class MooseMesh;

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
  MooseMesh & _mesh;
  std::string _var_name;
  unsigned int _nodesetid;

  std::vector<unsigned int> _node_ids;
};

#endif //AVERAGENODALVARIABLEVALUE_H_
