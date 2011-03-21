#ifndef PRINTDT_H_
#define PRINTDT_H_

#include "GeneralPostprocessor.h"

//Forward Declarations
class PrintDT;

template<>
InputParameters validParams<PrintDT>();

class PrintDT : public GeneralPostprocessor
{
public:
  PrintDT(const std::string & name, InputParameters parameters);
  
  virtual void initialize() {}
  virtual void execute() {}

  /**
   * This will return the degrees of freedom in the system.
   */
  virtual Real getValue();
};

#endif //PRINTDT_H_
