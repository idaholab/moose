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
         getParam<MultiMooseEnum>("constraint_meaning"),
         _temperature,
         getParam<unsigned>("extra_iterations_to_make_consistent"),
         getParam<Real>("min_initial_molality"),
         {},
         {}),
    _solver(_mgd,
            _egs,
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
  DenseMatrix<Real> dmole_additions(_num_basis, _num_basis);
  _solver.solveSystem(
      _solver_output, _tot_iter, _abs_residual, 0.0, _mole_additions, dmole_additions);
}

const GeochemicalSystem &
GeochemistryTimeIndependentReactor::getGeochemicalSystem(const Point & /*point*/) const
{
  return _egs;
}

const GeochemicalSystem &
GeochemistryTimeIndependentReactor::getGeochemicalSystem(unsigned /*node_id*/) const
{
  return _egs;
}

const std::stringstream &
GeochemistryTimeIndependentReactor::getSolverOutput(const Point & /*point*/) const
{
  return _solver_output;
}

unsigned
GeochemistryTimeIndependentReactor::getSolverIterations(const Point & /*point*/) const
{
  return _tot_iter;
}

Real
GeochemistryTimeIndependentReactor::getSolverResidual(const Point & /*point*/) const
{
  return _abs_residual;
}

const DenseVector<Real> &
GeochemistryTimeIndependentReactor::getMoleAdditions(unsigned /*node_id*/) const
{
  return _mole_additions;
}

const DenseVector<Real> &
GeochemistryTimeIndependentReactor::getMoleAdditions(const Point & /*point*/) const
{
  return _mole_additions;
}

Real
GeochemistryTimeIndependentReactor::getMolesDumped(unsigned /*node_id*/,
                                                   const std::string & /*species*/) const
{
  return 0.0;
}
