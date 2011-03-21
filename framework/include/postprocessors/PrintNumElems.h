#ifndef PRINTELEMS_H_
#define PRINTELEMS_H_

#include "GeneralPostprocessor.h"

//Forward Declarations
class PrintNumElems;

template<>
InputParameters validParams<PrintNumElems>();

class PrintNumElems : public GeneralPostprocessor
{
public:
  PrintNumElems(const std::string & name, InputParameters parameters);
  
  virtual void initialize() {}
  
  virtual void execute() {}

  /**
   * This will return the number of elements in the system
   */
  virtual Real getValue();
};

#endif //PRINTELEMS_H_
