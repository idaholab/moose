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

  unsigned int _this_var_num;
  MaterialProperty<std::vector<unsigned int> > &_p_var_nums;

  MaterialProperty<std::vector<Real> > &_viscosity;
  MaterialProperty<RealVectorValue> &_gravity;
  MaterialProperty<RealTensorValue> & _permeability;
  MaterialProperty<std::vector<Real> > &_rel_perm;
  MaterialProperty<std::vector<Real> > &_density;
  Function & _func;
  FEProblem & _feproblem;
};

#endif // RICHARDSEXCAVFLOW_H
