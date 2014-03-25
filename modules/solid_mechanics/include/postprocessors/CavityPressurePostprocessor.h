#ifndef CAVITYPRESSUREPOSTPROCESSOR_H
#define CAVITYPRESSUREPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

class CavityPressureUserObject;

class CavityPressurePostprocessor : public GeneralPostprocessor
{
public:

  CavityPressurePostprocessor(const std::string & name, InputParameters parameters);

  virtual ~CavityPressurePostprocessor(){}

  virtual void initialize() {}

  virtual void execute() {}

  virtual PostprocessorValue getValue();

protected:

  const CavityPressureUserObject & _cpuo;

  const std::string _quantity;

};

template<>
InputParameters validParams<CavityPressurePostprocessor>();

#endif
