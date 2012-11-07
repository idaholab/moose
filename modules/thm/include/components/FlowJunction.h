#ifndef FLOWJUNCTION_H
#define FLOWJUNCTION_H

#include <string>

#include "Joint.h"

class FlowJunction;

template<>
InputParameters validParams<FlowJunction>();

/**
 * Flow junction (isothermal)
 */
class FlowJunction : public Joint
{
public:
  FlowJunction(const std::string & name, InputParameters params);
  virtual ~FlowJunction();

  virtual void buildMesh();
  virtual void addVariables();
  virtual void addMooseObjects();

protected:
  std::vector<std::string> _inputs;
  std::vector<std::string> _outputs;
  std::vector<unsigned int> _bnd_id;

  //std::string _density_var_name;	///< Flow flow junction maintains one scalar variable (density of this flow junction)
  //std::string _specific_int_energy_var_name;	///< An optional variable, if non-isothermal

  std::string _FlowJunction_var_name;

  /// A vector to store user inputed K loss (form loss, minor loss) coefficients.
  std::vector<Real> _k_coeffs;
  /// A reference area for this flow junction to calculate its reference velocity (User input)
  Real _ref_area;
  /// A reference volume for this flow junction
  Real _ref_volume;

  /// Is initial pressure provided from user input
  bool _has_initial_P;
  /// Is initial temperature provided from user input
  bool _has_initial_T;
  /// Initial pressure from user input (if provided)
  Real _initial_P;
  /// Initial temperature from user input (if provided)
  Real _initial_T;
};

#endif /* FLOWJUNCTION_H */
