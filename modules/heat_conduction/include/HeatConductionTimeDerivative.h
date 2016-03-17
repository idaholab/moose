/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef HEATCONDUCTIONTIMEDERIVATIVE_H
#define HEATCONDUCTIONTIMEDERIVATIVE_H

// MOOSE includes
#include "TimeDerivative.h"
#include "Material.h"

// Forward Declarations
class HeatConductionTimeDerivative;

template<>
InputParameters validParams<HeatConductionTimeDerivative>();

/**
 * A class for defining the time derivative of the heat equation.
 *
 * By default this Kernel computes:
 *   \f$ \rho * c_p * \frac{\partial T}{\partial t}, \f$
 * where \f$ \rho \f$ and \f$ c_p \f$ are material properties with the names "density" and
 * "specific_heat", respectively.
 *
 * It is also possible to formulate the equation as:
 *   \f$ C_p * \frac{\partial T}{\partial t}, \f$
 * where \f$ C_p  \f$is a meterial property with the name "heat_capacity". To use
 * this formulation the parameter, "use_heat_capacity", must be set to true.
 */
class HeatConductionTimeDerivative : public TimeDerivative
{
public:
  /// Contructor for Heat Equation time derivative term.
  HeatConductionTimeDerivative(const InputParameters & parameters);

protected:
  /// Compute the residual of the Heat Equation time derivative.
  virtual Real computeQpResidual();

  /// Compute the jacobian of the Heat Equation time derivative.
  virtual Real computeQpJacobian();

  /**
   * Setup the material property for the correct formulation of the equation
   */
  virtual void initialSetup();

private:
  /// Flag that indicates the type of formulation to utilize
  bool _use_heat_capacity;

  /**
   * A material property set to unity for setting the density equal to one
   * This is used when using the heat capacity formulation
   */
  MaterialProperty<Real> _one;

  ///@{
  /**
   * Pointers to the specific heat and density properties.
   * These are pointers to allow for the adjustment of what properties are used. When
   * using the heat capacity formulation, _specific_heat is set the heat capacity and
   * density is equal to one.
   */
  const MaterialProperty<Real> * _specific_heat;
  const MaterialProperty<Real> * _density;
  ///@}
};

#endif //HEATCONDUCTIONTIMEDERIVATIVE_H
