#ifndef PRINTDOFS_H
#define PRINTDOFS_H

#include "Postprocessor.h"

//Forward Declarations
class PrintDOFs;

template<>
InputParameters validParams<PrintDOFs>();

class PrintDOFs : public Postprocessor
{
public:
  PrintDOFs(std::string name, MooseSystem &moose_system, InputParameters parameters);
  
  virtual void initialize() {}
  
  virtual void execute() {}

  /**
   * This will return the degrees of freedom in the system.
   */
  virtual Real getValue();
};

#endif //PRINTDOFS_H
