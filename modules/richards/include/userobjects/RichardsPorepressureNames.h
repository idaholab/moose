//  Holds maps between pressure_vars used in RichardsMaterial and the variable number used by MOOSE.
//
#ifndef RICHARDSPOREPRESSURENAMES_H
#define RICHARDSPOREPRESSURENAMES_H

#include "GeneralUserObject.h"
#include "Coupleable.h"

class RichardsPorepressureNames;


template<>
InputParameters validParams<RichardsPorepressureNames>();

class RichardsPorepressureNames :
public GeneralUserObject,
public Coupleable

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

 protected:
  unsigned int _num_p;
  std::string _the_names;
  std::vector<unsigned int> _moose_var_num;
  std::vector<VariableValue *> _moose_var_value;
  std::vector<unsigned int> _pressure_var_num;

};

#endif // RICHARDSPOREPRESSURENAMES_H
