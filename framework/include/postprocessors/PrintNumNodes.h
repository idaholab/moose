#ifndef PRINTNODES_H_
#define PRINTNODES_H_

#include "GeneralPostprocessor.h"

//Forward Declarations
class PrintNumNodes;

template<>
InputParameters validParams<PrintNumNodes>();

class PrintNumNodes : public GeneralPostprocessor
{
public:
  PrintNumNodes(const std::string & name, InputParameters parameters);
  
  virtual void initialize() {}
  virtual void execute() {}

  /**
   * This will return the number of nodes in the system
   */
  virtual Real getValue();
};

#endif //PRINTNODES_H_
