/*************************************************/
/*           DO NOT MODIFY THIS HEADER           */
/*                                               */
/*                     BISON                     */
/*                                               */
/*    (c) 2015 Battelle Energy Alliance, LLC     */
/*            ALL RIGHTS RESERVED                */
/*                                               */
/*   Prepared by Battelle Energy Alliance, LLC   */
/*     Under Contract No. DE-AC07-05ID14517      */
/*     With the U. S. Department of Energy       */
/*                                               */
/*     See COPYRIGHT for full restrictions       */
/*************************************************/

#pragma once

#include "AddFunctionAction.h"

class ChemicalCompositionAction : public Action
{
public:
  static InputParameters validParams();
  ChemicalCompositionAction(InputParameters params);

  virtual void act();

protected:
  void readCSV();

  /// Element names
  std::vector<std::string> _elements;
  /// Varaible type
  const std::string _var_type;
  /// The file name with initial values stored in a CSV format
  const FileName & _inital_values_file_name;
  /// Initial conditions for each element: [element name] => initial condition value
  std::map<std::string, Real> _initial_conditions;
  FileName _thermoFile;
  std::string _tunit;
  std::string _punit;
  std::string _munit;
  std::vector<std::string> _phases;
  std::vector<std::string> _species;
  std::vector<std::string> _element_potentials;
};
