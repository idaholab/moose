/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef COUPLEDEXECUTIONER_H
#define COUPLEDEXECUTIONER_H

#include "Executioner.h"

class CoupledExecutioner;
class CoupledProblem;
class ActionWarehouse;
class FEProblem;
class Parser;


template<>
InputParameters validParams<CoupledExecutioner>();

/**
 *
 */
class CoupledExecutioner : public Executioner
{
public:
  CoupledExecutioner(const std::string & name, InputParameters parameters);
  virtual ~CoupledExecutioner();

  virtual Problem & problem();

  // Interface for adding coupled problems /////

  /**
   * Add an FEProblem to the system of coupled problems
   * @param name The name of the FE problem
   * @param file_name The file name of the input file
   */
  virtual void addFEProblem(const std::string & name, const FileName & file_name);

  /**
   * Add a mapping how variables are projected from problems to problems
   * @param dest Name of the target problem (the one we are projecting into)
   * @param dest_var_name Name of the variable in the target problem
   * @param src Name of the source problem (the one we are projecting from)
   * @param src_var_name Name of the variable in the source problem
   */
  virtual void addCoupledVariable(const std::string & dest, const std::string & dest_var_name, const std::string & src, const std::string & src_var_name);

  /**
   * Build the entire problem
   */
  virtual void build();

protected:
  struct ProjInfo
  {
    /// Name of the source problem
    std::string src;
    /// Variable name in the source problem
    std::string src_var;
    /// Variable name in the destination problem
    std::string dest_var;
  };

  /// Instance of CoupledProblem class
  CoupledProblem * _problem;
  /// Mapping: FE problem name -> index to _awhs | _parsers arrays. After build() also _executioners | _fe_problems
  std::map<std::string, unsigned int> _name_index;
  /// Action warehouses to store actions into
  std::vector<ActionWarehouse *> _awhs;
  /// Parsers for creating the actions (have to keep them around since some actions are accessing them when executed)
  std::vector<Parser *> _parsers;
  /// Executioners build from input files
  std::vector<Executioner *> _executioners;
  /// FE problems build from input files
  std::vector<FEProblem *> _fe_problems;
  /// Variable mapping: problem name -> list variables that needs to be projected
  std::map<std::string, std::vector<ProjInfo *> > _var_mapping;
  /// FE Problem mapping: fe_problem -> its name
  std::map<FEProblem *, std::string> _fep_mapping;

  /**
   * Create an action that adds a variable
   * @param task Type of action we are looking for
   * @param src Source warehouse that is search for actions that satisfy task
   * @param src_var_name Name of the variable in the source problem
   * @param dest Target warehouse we put the new action into
   * @param dest_var_name Name of the variable in the target problem
   */
  void addVariableAction(const std::string & task, ActionWarehouse & src, const std::string & src_var_name, ActionWarehouse & dest, const std::string & dest_var_name);

  /**
   * Project all variables that problem 'fep' needs
   * @param fep
   */
  void projectVariables(FEProblem & fep);

  /**
   * Get problem by its name (the name that was specified in an input file)
   * @param name Name of the problem we want to get
   * @return Pointer to the problem
   */
  virtual FEProblem * getProblemByName(const std::string & name);

  virtual Executioner * getExecutionerByName(const std::string & name);
};


#endif /* COUPLEDEXECUTIONER_H */
