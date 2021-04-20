//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemistryTimeIndependentReactor.h"

registerMooseObject("GeochemistryApp", GeochemistryTimeIndependentReactor);

InputParameters
GeochemistryTimeIndependentReactor::validParams()
{
  InputParameters params = GeochemistryReactorBase::validParams();
  params.addParam<Real>("temperature", 25.0, "The temperature (degC) of the aqueous solution");
  params.addClassDescription("UserObject that controls the time-independent geochemistry reaction "
                             "processes.  Spatial dependence is not possible using this class");
  params.set<ExecFlagEnum>("execute_on") = {EXEC_FINAL};
  return params;
}

GeochemistryTimeIndependentReactor::GeochemistryTimeIndependentReactor(
    const InputParameters & parameters)
  : GeochemistryReactorBase(parameters),
    _temperature(getParam<Real>("temperature")),
    _egs(_mgd,
         _gac,
         _is,
         _swapper,
         getParam<std::vector<std::string>>("swap_out_of_basis"),
         getParam<std::vector<std::string>>("swap_into_basis"),
         getParam<std::string>("charge_balance_species"),
         getParam<std::vector<std::string>>("constraint_species"),
         getParam<std::vector<Real>>("constraint_value"),
         getParam<MultiMooseEnum>("constraint_unit"),
         getParam<MultiMooseEnum>("constraint_meaning"),
         _temperature,
         getParam<unsigned>("extra_iterations_to_make_consistent"),
         getParam<Real>("min_initial_molality"),
         {},
         {},
         MultiMooseEnum("")),
    _solver(_mgd.basis_species_name.size(),
            _mgd.kin_species_name.size(),
            _is,
            getParam<Real>("abs_tol"),
            getParam<Real>("rel_tol"),
            getParam<unsigned>("max_iter"),
            getParam<Real>("max_initial_residual"),
            _small_molality,
            _max_swaps_allowed,
            getParam<std::vector<std::string>>("prevent_precipitation"),
            getParam<Real>("max_ionic_strength"),
            getParam<unsigned>("ramp_max_ionic_strength_initial"),
            false),
    _mole_additions(DenseVector<Real>(_num_basis, 0.0))
{
}

void
GeochemistryTimeIndependentReactor::initialize()
{
  GeochemistryReactorBase::initialize();
}
void
GeochemistryTimeIndependentReactor::finalize()
{
  GeochemistryReactorBase::finalize();
}

void
GeochemistryTimeIndependentReactor::execute()
{
  GeochemistryReactorBase::execute();
}

void
GeochemistryTimeIndependentReactor::initialSetup()
{
  if (_num_my_nodes == 0)
    return; // rather peculiar case where user has used many processors
  DenseMatrix<Real> dmole_additions(_num_basis, _num_basis);
  _solver.solveSystem(_egs,
                      _solver_output[0],
                      _tot_iter[0],
                      _abs_residual[0],
                      0.0,
                      _mole_additions,
                      dmole_additions);
}

const GeochemicalSystem &
    GeochemistryTimeIndependentReactor::getGeochemicalSystem(dof_id_type /*node_id*/) const
{
  return _egs;
}

const std::stringstream &
    GeochemistryTimeIndependentReactor::getSolverOutput(dof_id_type /*node_id*/) const
{
  return _solver_output[0];
}

unsigned GeochemistryTimeIndependentReactor::getSolverIterations(dof_id_type /*node_id*/) const
{
  return _tot_iter[0];
}

Real GeochemistryTimeIndependentReactor::getSolverResidual(dof_id_type /*node_id*/) const
{
  return _abs_residual[0];
}

const DenseVector<Real> &
    GeochemistryTimeIndependentReactor::getMoleAdditions(dof_id_type /*node_id*/) const
{
  return _mole_additions;
}

Real
GeochemistryTimeIndependentReactor::getMolesDumped(dof_id_type /*node_id*/,
                                                   const std::string & /*species*/) const
{
  return 0.0;
}
