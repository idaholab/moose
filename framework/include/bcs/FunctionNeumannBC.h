#ifndef FUNCTIONNEUMANNBC_H
#define FUNCTIONNEUMANNBC_H

#include "BoundaryCondition.h"

//Forward Declarations
class FunctionNeumannBC;
class Function;

template<>
InputParameters validParams<FunctionNeumannBC>();


class FunctionNeumannBC : public BoundaryCondition
{
public:

  FunctionNeumannBC(std::string name,
             MooseSystem &sys,
             InputParameters parameters);

protected:

  virtual Real computeQpResidual();

private:
  Function & _func;
};
#endif //FUNCTIONNEUMANNBC_H
