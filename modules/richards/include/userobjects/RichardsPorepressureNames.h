/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#ifndef RICHARDSPOREPRESSURENAMES_H
#define RICHARDSPOREPRESSURENAMES_H

#include "GeneralUserObject.h"
#include "Coupleable.h"
#include "ZeroInterface.h"

class RichardsPorepressureNames;


template<>
InputParameters validParams<RichardsPorepressureNames>();

/**
 * This holds maps between pressure_vars used in RichardsMaterial
 * and kernels, etc, and the variable number used internally by
 * MOOSE
 */
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

  /// the number of porepressure variables
  unsigned int num_pp() const;

  /**
   * the MOOSE variable number
   * @param pressure_var_num the pressure variable number
   * eg if porepressure_vars = 'pwater pgas', and the variables in
   * the simulation are 'temperature pwater pgas displacement'
   * then moose_var_num(0) = 1
   */
  unsigned int moose_var_num(unsigned int pressure_var_num) const;

  /**
   * the pressure variable number
   * @param moose_var_num the MOOSE variable number
   * eg if porepressure_vars = 'pwater pgas', and the variables in
   * the simulation are 'temperature pwater pgas displacement'
   * then pressure_var_num(2) = 1
   */
  unsigned int pressure_var_num(unsigned int moose_var_num) const;

  /**
   * returns true if moose_var_num is not a porepressure
   * @param moose_var_num the MOOSE variable number
   * eg if porepressure_vars = 'pwater pgas', and the variables in
   * the simulation are 'temperature pwater pgas displacement'
   * then not_pressure_var(0) = true, no_pressure_var(1) = false
   */
  bool not_pressure_var(unsigned int moose_var_num) const;

  /**
   * a space-separated string of porepressure names
   * eg if pp_names() = 'pwater pgas'
   */
  std::string pp_names() const;

  /**
   * a vector of pointers to VariableValues
   * @param pressure_var_num the pressure variable number
   * eg if porepressure_vars = 'pwater pgas', then
   * (*pp_vals(1))[qp] = pgas evaluated at quadpoint qp
   * Also pp_vals(i) = &coupledValue
   */
  VariableValue * pp_vals(unsigned int pressure_var_num) const;

  /**
   * a vector of pointers to old VariableValues
   * @param pressure_var_num the pressure variable number
   * eg if porepressure_vars = 'pwater pgas', then
   * (*pp_vals_old(1))[qp] = old pgas evaluated at quadpoint qp
   * Also pp_vals_old(i) = &coupledValueOld
   */
  VariableValue * pp_vals_old(unsigned int pressure_var_num) const;

  /**
   * a vector of pointers to grad(Variable)
   * @param pressure_var_num the pressure variable number
   * eg if porepressure_vars = 'pwater pgas', then
   * (*grad_pp(1))[qp] = grad(pgas) evaluated at quadpoint qp
   * Also grad_pp(i) = &coupledGradient
   */
  VariableGradient * grad_pp(unsigned int pressure_var_num) const;

  /**
   * The moose variable for the given pressure_var_num
   * This is got using the getVar function.  It allows
   * direct extraction of nodal porepressure values
   * used in mass lumping.
   * @param pressure_var_num the pressure variable number
   */
  MooseVariable * raw_pp(unsigned int pressure_var_num) const;

 protected:

  /// number of porepressure variables
  unsigned int _num_p;

  /// space-separated string of names of porepressure variables
  std::string _the_names;

  /// _moose_var_num[i] = the moose variable number corresponding to porepressure i
  std::vector<unsigned int> _moose_var_num;

  /// _pressure_var_num[i] = the pressure variable corresponding to moose variable i
  std::vector<unsigned int> _pressure_var_num;

  /// moose_var_value[i] = values of porepressure i
  std::vector<VariableValue *> _moose_var_value; // this is a vector of pointers to VariableValues

  /// moose_var_value_old[i] = old values of porepressure i
  std::vector<VariableValue *> _moose_var_value_old;

  /// moose_grad_var[i] = gradient values of porepressure i
  std::vector<VariableGradient *> _moose_grad_var;

  /// _moose_raw_var[i] = getVar of porepressure i
  std::vector<MooseVariable *> _moose_raw_var;

};

#endif // RICHARDSPOREPRESSURENAMES_H
