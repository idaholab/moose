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

  const std::vector<Real> &_k_coeffs; ///< A vector to store user inputed K loss (form loss, minor loss) coefficients.
  const Real &_ref_area;              ///< A reference area for this flow junction to calculate its reference velocity (User input)
  const Real &_ref_volume;            ///< A reference volume for this flow junction

  bool _has_initial_P;                ///< Is initial pressure provided from user input
  bool _has_initial_T;                ///< Is initial temperature provided from user input
  const Real &_initial_P;             ///< Initial pressure from user input (if provided)
  const Real &_initial_T;             ///< Initial temperature from user input (if provided)
};

#endif /* FLOWJUNCTION_H */
