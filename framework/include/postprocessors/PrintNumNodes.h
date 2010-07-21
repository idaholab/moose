#ifndef PRINTNODES_H
#define PRINTNODES_H

#include "Postprocessor.h"

//Forward Declarations
class PrintNumNodes;

template<>
InputParameters validParams<PrintNumNodes>();

class PrintNumNodes : public Postprocessor
{
public:
  PrintNumNodes(std::string name, MooseSystem &moose_system, InputParameters parameters);
  
  virtual void initialize() {}
  
  virtual void execute() {}

  /**
   * This will return the number of nodes in the system
   */
  virtual Real getValue();
};

#endif //PRINTNODES_H
