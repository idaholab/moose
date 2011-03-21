#ifndef FUNCTIONNEUMANNBC_H_
#define FUNCTIONNEUMANNBC_H_

#include "IntegratedBC.h"

//Forward Declarations
class FunctionNeumannBC;
class Function;

template<>
InputParameters validParams<FunctionNeumannBC>();


class FunctionNeumannBC : public IntegratedBC
{
public:
  FunctionNeumannBC(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  Function & _func;
};

#endif // FUNCTIONNEUMANNBC_H_
