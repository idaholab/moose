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

#include "AuxKernel.h"
#include "ThermochimicaNodalData.h"

class ThermochimicaAux : public AuxKernel
{
public:
  static InputParameters validParams();
  ThermochimicaAux(const InputParameters & parameters);

  virtual ~ThermochimicaAux() {}

protected:
  virtual Real computeValue();

  unsigned int _n_phases;
  std::vector<VariableValue *> _ph;
  const bool _phases_coupled;
  std::vector<std::string> _ph_name;

  unsigned int _n_species;
  std::vector<VariableValue *> _sp;
  const bool _species_coupled;
  std::vector<std::string> _sp_phase_name;
  std::vector<std::string> _sp_species_name;

  const bool _output_element_potential;
  std::vector<VariableValue *> _el_pot;

private:
  const ThermochimicaNodalData * _thermo_nodal_data_uo;
};
