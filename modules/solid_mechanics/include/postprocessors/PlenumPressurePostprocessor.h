#ifndef PLENUMPRESSUREPOSTPROCESSOR_H
#define PLENUMPRESSUREPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

class PlenumPressureUserObject;

class PlenumPressurePostprocessor : public GeneralPostprocessor
{
public:

  PlenumPressurePostprocessor(const std::string & name, InputParameters parameters);

  virtual ~PlenumPressurePostprocessor(){}

  virtual void initialize() {}

  virtual void execute() {}

  virtual PostprocessorValue getValue();

protected:

  const PlenumPressureUserObject & _ppuo;

  const std::string _quantity;

};

template<>
InputParameters validParams<PlenumPressurePostprocessor>();

#endif
