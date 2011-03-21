#ifndef REPORTER_H_
#define REPORTER_H_

#include "GeneralPostprocessor.h"

//Forward Declarations
class Reporter;

template<>
InputParameters validParams<Reporter>();

class Reporter : public GeneralPostprocessor
{
public:
  Reporter(const std::string & name, InputParameters parameters);

  virtual void initialize() {}
  virtual void execute() {}

  /**
   * This will return the degrees of freedom in the system.
   */
  virtual Real getValue();

protected:
  const PostprocessorValue & _my_value;
};

#endif //REPORTER_H
