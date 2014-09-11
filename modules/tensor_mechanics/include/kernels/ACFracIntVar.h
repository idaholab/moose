#ifndef ACFracIntVar_H
#define ACFracIntVar_H

#include "KernelValue.h"

//Forward Declarations
class ACFracIntVar;

template<>
InputParameters validParams<ACFracIntVar>();

/**
 * Formulation: Miehe et. al., Int. J. Num. Methods Engg., 2010, 83. 1273-1311
 * Equation 63
 */
class ACFracIntVar : public KernelValue
{
public:
  ACFracIntVar(const std::string & name, InputParameters parameters);

protected:
  VariableValue & _c;

  enum PFFunctionType
  {
    Jacobian,
    Residual
  };

  virtual Real precomputeQpResidual();
  virtual Real precomputeQpJacobian();
};

#endif //ACBulk_H
