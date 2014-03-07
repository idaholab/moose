#ifndef BULKCOOLANTBC_H
#define BULKCOOLANTBC_H

#include "IntegratedBC.h"
#include "HeatConductionMaterial.h"
#include "Function.h"

//Forward Declarations
class BulkCoolantBC;

template<>
InputParameters validParams<BulkCoolantBC>();

class BulkCoolantBC : public IntegratedBC
{
public:

  BulkCoolantBC(const std::string & name, InputParameters parameters);

  virtual ~BulkCoolantBC(){}

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

private:
  Real _alpha;
  Real _tempb;
  const bool _has_function;
  Function * _function;
  MaterialProperty<Real> & _conductivity;

};

#endif //BULKCOOLANTBC_H
