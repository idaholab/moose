#ifndef FLOWJUNCTION_H
#define FLOWJUNCTION_H

#include <string>

#include "Junction.h"

class FlowJunction;

template<>
InputParameters validParams<FlowJunction>();

/**
 * Junction for flow
 */
class FlowJunction : public Junction
{
public:
  FlowJunction(const std::string & name, InputParameters params);
  virtual ~FlowJunction();

  virtual void addVariables();
  virtual void addMooseObjects();

  virtual std::vector<unsigned int> getIDs(std::string piece);
  virtual std::string variableName(std::string piece);

protected:
  /// Name of the SCALAR density variable in the junction
  std::string _junction_rho_name;

  /// Name of the SCALAR momentum variable in the junction
  std::string _junction_rhou_name;

  /// Name of the SCALAR total energy variable in the junction
  std::string _junction_rhoE_name;

  /// Name of the coupled scalar aux variable for the pressure.
  std::string _p_name;

  /// Name of the coupled scalar aux variable for the temperature.
  std::string _T_name;

  /// Volume of the junction
  Real _junction_vol;

  /// Gravity in the junction
  Real _junction_gravity;

  /// Loss in the junction
  Real _junction_loss;

  /// Area of the junction
  Real _junction_area;

  /// Initial density in the junction
  Real _initial_rho_junction;

  /// Initial momentum in the junction
  Real _initial_rhou_junction;

  /// Initial total energy in the junction
  Real _initial_rhoE_junction;
};

#endif /* FLOWJUNCTION_H */
