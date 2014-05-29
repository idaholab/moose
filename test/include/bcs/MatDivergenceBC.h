#ifndef MATDIVERGENCEBC_H
#define MATDIVERGENCEBC_H

#include "DivergenceBC.h"

class MatDivergenceBC;

template<>
InputParameters validParams<MatDivergenceBC>();

/**
 * Extends DivergenceBC by multiplication of material property
 */
class MatDivergenceBC : public DivergenceBC
{
public:
  MatDivergenceBC(const std::string & name, InputParameters parameters);
  virtual ~MatDivergenceBC();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  MaterialProperty<Real> & _mat;
};

#endif /* MATDIVERGENCEBC_H */
