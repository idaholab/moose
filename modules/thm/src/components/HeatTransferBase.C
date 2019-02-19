#include "HeatTransferBase.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"
#include "Pipe.h"
#include "MooseUtils.h"

template <>
InputParameters
validParams<HeatTransferBase>()
{
  InputParameters params = validParams<ConnectorBase>();

  params.addRequiredParam<std::string>("pipe", "Name of pipe component to connect");

  params.addParam<bool>(
      "P_hf_transferred", false, "Is heat flux perimeter transferred from an external source?");
  params.addParam<FunctionName>("P_hf", "Heat flux perimeter function");

  params.addParam<FunctionName>("Hw", "Convective heat transfer coefficient");
  params.addParam<FunctionName>("Hw_liquid",
                                "Convective one-phase liquid heat transfer coefficient");
  params.addParam<FunctionName>("Hw_vapor", "Convective one-phase vapor heat transfer coefficient");

  return params;
}

HeatTransferBase::HeatTransferBase(const InputParameters & parameters)
  : ConnectorBase(parameters),

    _pipe_name(getParam<std::string>("pipe")),

    _P_hf_transferred(getParam<bool>("P_hf_transferred")),
    _P_hf_provided(isParamValid("P_hf")),

    _phase_interaction(false)
{
  addDependency(_pipe_name);
}

void
HeatTransferBase::init()
{
  ConnectorBase::init();

  checkComponentOfTypeExistsByName<Pipe>(_pipe_name);

  if (hasComponentByName<Pipe>(_pipe_name))
  {
    const Pipe & pipe = getComponentByName<Pipe>(_pipe_name);

    // add the name of this heat transfer component to list for pipe
    pipe.addHeatTransferName(name());

    // get various data from pipe
    _block_ids_pipe = pipe.getSubdomainIds();
    _model_type = pipe.getFlowModelID();
    if (_model_type != THM::FM_SINGLE_PHASE)
    {
      auto flow_model = pipe.getFlowModel();
      auto flow_model_2phase = dynamic_cast<const FlowModelTwoPhase &>(*flow_model);
      _phase_interaction = flow_model_2phase.getPhaseInteraction();
    }
    _fp_name = pipe.getFluidPropertiesName();
    _A_fn_name = pipe.getAreaFunctionName();
    _closures_name = MooseUtils::toLower(pipe.getParam<MooseEnum>("closures_type"));
  }
}

void
HeatTransferBase::initSecondary()
{
  ConnectorBase::initSecondary();

  // determine names of heat transfer variables
  if (hasComponentByName<Pipe>(_pipe_name))
  {
    const Pipe & pipe = getComponentByName<Pipe>(_pipe_name);

    const std::string suffix = pipe.getHeatTransferNamesSuffix(name());
    const std::string Hw_suffix = _closures_name == "simple" ? suffix : "";

    _Hw_1phase_name = FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL + Hw_suffix;
    _Hw_liquid_name = FlowModelTwoPhase::HEAT_TRANSFER_COEFFICIENT_WALL_LIQUID + Hw_suffix;
    _Hw_vapor_name = FlowModelTwoPhase::HEAT_TRANSFER_COEFFICIENT_WALL_VAPOR + Hw_suffix;
    _P_hf_name = FlowModel::HEAT_FLUX_PERIMETER + suffix;
    _T_wall_name = FlowModel::TEMPERATURE_WALL + suffix;
    _q_wall_name = FlowModel::HEAT_FLUX_WALL + suffix;
  }
}

void
HeatTransferBase::check() const
{
  ConnectorBase::check();

  if (_model_type == THM::FM_SINGLE_PHASE)
  {
    if (_closures_name == "simple")
    {
      if (!isParamValid("Hw"))
        logError("The parameter 'Hw' must be provided when using simple closures.");
    }
    else if (_closures_name == "trace")
    {
      if (isParamValid("Hw"))
        logError("The parameter 'Hw' cannot be provided when using TRACE closures.");
    }
  }
  else if (_model_type == THM::FM_TWO_PHASE || _model_type == THM::FM_TWO_PHASE_NCG)
  {
    if (_closures_name == "simple")
    {
      if (!(isParamValid("Hw_liquid") && isParamValid("Hw_vapor")))
        logError("The parameters 'Hw_liquid' and 'Hw_vapor' must be provided"
                 " when using simple closures.");
    }
    else if (_closures_name == "trace")
    {
      if (isParamValid("Hw_liquid") && isParamValid("Hw_vapor"))
        logError("The parameters 'Hw_liquid' and 'Hw_vapor' cannot be provided"
                 " when using TRACE closures.");
    }
  }
}

void
HeatTransferBase::addVariables()
{
  // heat flux perimeter variable
  if (!_P_hf_transferred)
    addHeatedPerimeter();
}

void
HeatTransferBase::addMooseObjects()
{
  addMooseObjectsCommon();
  if (_model_type == THM::FM_SINGLE_PHASE)
    addMooseObjects1Phase();
  else if (_model_type == THM::FM_TWO_PHASE || _model_type == THM::FM_TWO_PHASE_NCG)
    addMooseObjects2Phase();
}

void
HeatTransferBase::addHeatedPerimeter()
{
  _sim.addVariable(false, _P_hf_name, FlowModel::feType(), _block_ids_pipe);

  // create heat flux perimeter variable if not transferred from external app
  if (!_P_hf_transferred)
  {
    if (_P_hf_provided)
    {
      _P_hf_fn_name = getParam<FunctionName>("P_hf");
    }
    // create heat flux perimeter function if not provided; assume circular pipe
    else
    {
      _P_hf_fn_name = genName(name(), "P_hf_fn");

      const std::string class_name = "GeneralizedCircumference";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<FunctionName>("area_function") = _A_fn_name;
      _sim.addFunction(class_name, _P_hf_fn_name, params);

      makeFunctionControllableIfConstant(_P_hf_fn_name, "P_hf");
    }

    _sim.addFunctionIC(_P_hf_name, _P_hf_fn_name, _pipe_name);
  }
}

void
HeatTransferBase::addMooseObjectsCommon()
{
  // create heat flux perimeter aux if not transferred from external app
  if (!_P_hf_transferred)
  {
    const std::string class_name = "FunctionAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = {_P_hf_name};
    params.set<std::vector<SubdomainName>>("block") = {_pipe_name};
    params.set<FunctionName>("function") = _P_hf_fn_name;

    ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
    execute_on = {EXEC_TIMESTEP_BEGIN, EXEC_INITIAL};
    params.set<ExecFlagEnum>("execute_on") = execute_on;

    _sim.addAuxKernel(class_name, genName(name(), "P_hf_auxkernel"), params);
  }
}

void
HeatTransferBase::addMooseObjects1Phase()
{
  if (_closures_name == "simple")
  {
    const FunctionName & Hw_fn_name = getParam<FunctionName>("Hw");

    {
      const std::string class_name = "GenericFunctionMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<SubdomainName>>("block") = {_pipe_name};
      params.set<std::vector<std::string>>("prop_names") = {_Hw_1phase_name};
      params.set<std::vector<FunctionName>>("prop_values") = {Hw_fn_name};
      _sim.addMaterial(class_name, genName(name(), "Hw_material"), params);
    }

    makeFunctionControllableIfConstant(Hw_fn_name, "Hw");
  }
}

void
HeatTransferBase::addMooseObjects2Phase()
{
  if (_closures_name == "simple")
  {
    const FunctionName & Hw_liquid = getParam<FunctionName>("Hw_liquid");
    const FunctionName & Hw_vapor = getParam<FunctionName>("Hw_vapor");

    {
      const std::string class_name = "GenericFunctionMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<SubdomainName>>("block") = {_pipe_name};
      params.set<std::vector<std::string>>("prop_names") = {_Hw_liquid_name, _Hw_vapor_name};
      params.set<std::vector<FunctionName>>("prop_values") = {Hw_liquid, Hw_vapor};
      _sim.addMaterial(class_name, genName(name(), "Hw_material"), params);
    }

    makeFunctionControllableIfConstant(Hw_liquid, "Hw_liquid");
    makeFunctionControllableIfConstant(Hw_vapor, "Hw_vapor");
  }
}

const MaterialPropertyName &
HeatTransferBase::getWallHeatTransferCoefficient1PhaseName() const
{
  checkSetupStatus(INITIALIZED_SECONDARY);

  return _Hw_1phase_name;
}

const MaterialPropertyName &
HeatTransferBase::getWallHeatTransferCoefficientLiquidName() const
{
  checkSetupStatus(INITIALIZED_SECONDARY);

  return _Hw_liquid_name;
}

const MaterialPropertyName &
HeatTransferBase::getWallHeatTransferCoefficientVaporName() const
{
  checkSetupStatus(INITIALIZED_SECONDARY);

  return _Hw_vapor_name;
}

const VariableName &
HeatTransferBase::getHeatedPerimeterName() const
{
  checkSetupStatus(INITIALIZED_SECONDARY);

  return _P_hf_name;
}

const VariableName &
HeatTransferBase::getWallTemperatureName() const
{
  checkSetupStatus(INITIALIZED_SECONDARY);

  return _T_wall_name;
}

const VariableName &
HeatTransferBase::getWallHeatFluxName() const
{
  return _q_wall_name;
}
