#pragma once

#include "NodalVariablePostprocessor.h"

class NodalMaxFromAverage;

template <>
InputParameters validParams<NodalMaxFromAverage>();

/**
 *
 */
class NodalMaxFromAverage : public NodalVariablePostprocessor
{
public:
  NodalMaxFromAverage(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();
  virtual void threadJoin(const UserObject & y);

protected:
  Real _value;
  std::string _average_name;
  const PostprocessorValue & _average;
};
