#ifndef DIFFERENCEPOSTPROCESSOR_H
#define DIFFERENCEPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

class DifferencePostprocessor;

template<>
InputParameters validParams<DifferencePostprocessor>();

/**
 * Computes the difference between two postprocessors
 *
 * result = value1 - value2
 */
class DifferencePostprocessor : public GeneralPostprocessor
{
public:
  DifferencePostprocessor(const std::string & name, InputParameters parameters);
  virtual ~DifferencePostprocessor();

  virtual void initialize();
  virtual void execute();
  virtual PostprocessorValue getValue();
  virtual void threadJoin(const UserObject & uo);

protected:
  const PostprocessorValue & _value1;
  const PostprocessorValue & _value2;
};


#endif /* DIFFERENCEPOSTPROCESSOR_H */
