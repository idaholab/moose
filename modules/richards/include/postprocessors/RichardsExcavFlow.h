/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSEXCAVFLOW_H
#define RICHARDSEXCAVFLOW_H

#include "SideIntegralVariablePostprocessor.h"
#include "MaterialPropertyInterface.h"
#include "FunctionInterface.h"
#include "RichardsPorepressureNames.h"

//Forward Declarations
class RichardsExcavFlow;
class Function;

template<>
InputParameters validParams<RichardsExcavFlow>();

/**
 * Records total mass flow into an excavation defined by a RichardsExcavGeom function
 */
class RichardsExcavFlow :
public SideIntegralVariablePostprocessor,
public FunctionInterface
{
public:
  RichardsExcavFlow(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpIntegral();

  /// holds info regarding the porepressure variables
  const RichardsPorepressureNames & _pp_name_UO;

  /// the porepressure number for which we want the mass flow
  unsigned int _pvar;

  /// viscosities of fluids
  MaterialProperty<std::vector<Real> > &_viscosity;

  /// gravitational acceleration vector
  MaterialProperty<RealVectorValue> &_gravity;

  /// permeability of medium
  MaterialProperty<RealTensorValue> & _permeability;

  /// relative permeabilites of fluids
  MaterialProperty<std::vector<Real> > &_rel_perm;

  /// densities of fluids
  MaterialProperty<std::vector<Real> > &_density;

  /// the RichardsExcavGeom that defines where on the boundary we'll compute the mass flux
  Function & _func;
};

#endif // RICHARDSEXCAVFLOW_H
