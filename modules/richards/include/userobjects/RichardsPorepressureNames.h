/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  Holds maps between pressure_vars used in RichardsMaterial and the variable number used by MOOSE.
//
#ifndef RICHARDSPOREPRESSURENAMES_H
#define RICHARDSPOREPRESSURENAMES_H

#include "GeneralUserObject.h"
#include "Coupleable.h"
#include "ZeroInterface.h"

class RichardsPorepressureNames;


template<>
InputParameters validParams<RichardsPorepressureNames>();

class RichardsPorepressureNames :
public GeneralUserObject,
public Coupleable,
public ZeroInterface


{
 public:
  RichardsPorepressureNames(const std::string & name, InputParameters parameters);

  void initialize();
  void execute();
  void finalize();

  unsigned int num_pp() const;
  unsigned int moose_var_num(unsigned int pressure_var_num) const;
  unsigned int pressure_var_num(unsigned int moose_var_num) const;
  bool not_pressure_var(unsigned int moose_var_num) const;
  std::string pp_names() const;
  VariableValue * pp_vals(unsigned int pressure_var_num) const;
  VariableValue * pp_vals_old(unsigned int pressure_var_num) const;
  VariableGradient * grad_pp(unsigned int pressure_var_num) const;

 protected:
  unsigned int _num_p;
  std::string _the_names;
  std::vector<unsigned int> _moose_var_num;
  std::vector<unsigned int> _pressure_var_num;
  std::vector<VariableValue *> _moose_var_value; // this is a vector of pointers to VariableValues
  std::vector<VariableValue *> _moose_var_value_old;
  std::vector<VariableGradient *> _moose_grad_var;

};

#endif // RICHARDSPOREPRESSURENAMES_H
