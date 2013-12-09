#ifndef RICHARDSEXCAVFLOW_H
#define RICHARDSEXCAVFLOW_H

#include "SideIntegralVariablePostprocessor.h"
#include "MaterialPropertyInterface.h"
#include "FunctionInterface.h"

//Forward Declarations
class RichardsExcavFlow;
class Function;

template<>
InputParameters validParams<RichardsExcavFlow>();

class RichardsExcavFlow : 
public SideIntegralVariablePostprocessor,
public FunctionInterface
{
public:
  RichardsExcavFlow(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpIntegral();

  MaterialProperty<Real> &_dens0;
  MaterialProperty<Real> &_viscosity;
  MaterialProperty<RealVectorValue> &_gravity;
  MaterialProperty<RealTensorValue> & _permeability;
  MaterialProperty<Real> &_rel_perm;
  MaterialProperty<Real> &_density;
  Function & _func;
  FEProblem & _feproblem;
};

#endif // RICHARDSEXCAVFLOW_H
