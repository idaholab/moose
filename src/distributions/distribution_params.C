/*
 * distribution_params.C
 *
 * Created on Nov 27, 2013
 *  Author cogljj (Joshua J Cogliati)
 */

#include "distribution_params.h"

#define throwError(msg) { std::cerr << "\n\n" << msg << "\n\n"; throw std::runtime_error("Error"); }

template<>
InputParameters validParams<distribution>(){

   InputParameters params = validParams<MooseObject>();

   params.addParam<double>("xMin", "Lower bound");
   params.addParam<double>("xMax", "Upper bound");

   params.addParam<double>("PB_window_Low", 0.0, "Probability window lower bound");
   params.addParam<double>("PB_window_Up" , 1.0, "Probability window upper bound");
   params.addParam<double>("V_window_Low" , -std::numeric_limits<double>::max( ), "Value window lower bound");
   params.addParam<double>("V_window_Up"  , std::numeric_limits<double>::max( ), "Value window upper bound");

   params.addParam<double>("ProbabilityThreshold", 1.0, "Probability Threshold");

   params.addParam<unsigned int>("seed", _defaultSeed ,"RNG seed");
   params.addRequiredParam<std::string>("type","distribution type");
   params.addParam<unsigned int>("truncation", 1 , "Type of truncation"); // Truncation types: 1) pdf_prime(x) = pdf(x)*c   2) [to do] pdf_prime(x) = pdf(x)+c
   //params.addParam<unsigned int>("force_distribution", 0 ,"force distribution to be evaluated at: if (0) Don't force distribution, (1) xMin, (2) Mean, (3) xMax");
   params.addParam<double>("force_probability","Force a specified probability to be used for getting random distribution numbers");
   params.addParam<double>("force_value","Force a specified value to be used for getting random distribution numbers");

   params.registerBase("distribution");
   return params;
}

class distribution;

distribution::distribution(const InputParameters & parameters):
      MooseObject(parameters)
{
   _type=getParam<std::string>("type");
   if(_type != "CustomDistribution"){
     if(parameters.isParamValid("xMin")) {
       _dist_parameters["xMin"] = getParam<double>("xMin");
     }

     if(parameters.isParamValid("xMax")) {
       _dist_parameters["xMax"] = getParam<double>("xMax");
     }

     if(parameters.isParamValid("force_probability")){
       setForcedConstant(getParam<double>("force_probability"));
       setForcingMethod(FORCED_PROBABILITY);
     }

     if(parameters.isParamValid("force_value")){
       setForcedConstant(getParam<double>("force_value"));
       setForcingMethod(FORCED_VALUE);
     }

   }
   else
   {
     std::vector<double> x_coordinates = getParam<std::vector<double> >("x_coordinates");
     _dist_parameters["xMin"] = x_coordinates[0];
     _dist_parameters["xMax"] = x_coordinates[x_coordinates.size()-1];
     std::vector<double> y_cordinates = getParam<std::vector<double> >("y_coordinates");
     //custom_dist_fit_type fitting_type = static_cast<custom_dist_fit_type>((int)getParam<MooseEnum>("fitting_type"));

     //_interpolation=InterpolationFunctions(x_coordinates,
     //                                       y_cordinates,
     //                                       fitting_type);
   }
      _seed = getParam<unsigned int>("seed");
      _dist_parameters["truncation"] = double(getParam<unsigned int>("truncation"));

      _dist_parameters["PB_window_Low"] = getParam<double>("PB_window_Low");
      _dist_parameters["PB_window_Up"]  = getParam<double>("PB_window_Up");

      _dist_parameters["V_window_Low"] = getParam<double>("V_window_Low");
      _dist_parameters["V_window_Up"]  = getParam<double>("V_window_Up");

      _dist_parameters["ProbabilityThreshold"] = getParam<double>("ProbabilityThreshold");

      // Data checks
      if (getParam<double>("PB_window_Low") >= getParam<double>("PB_window_Up"))
        throwError("Distribution 1D " << name() << " - PB window values wrong: Low > Up ");

      if (getParam<double>("V_window_Low") >= getParam<double>("V_window_Up"))
        throwError("Distribution 1D " << name() << " - V window values wrong: Low > Up ");

      if (getParam<double>("ProbabilityThreshold") > 1.0 || getParam<double>("ProbabilityThreshold") < 0.0)
        throwError("Distribution 1D " << name() << " - ProbabilityThreshold is not correct: it must be between 0.0 and 1.0 ");

      _checkStatus = false;
}

distribution::~distribution(){
}

/*
 * CLASS UNIFORM DISTRIBUTION
 */


template<>
InputParameters validParams<UniformDistribution>(){

   InputParameters params = validParams<distribution>();

   params.addRequiredParam<double>("xMin", "Distribution lower bound");
   params.addRequiredParam<double>("xMax", "Distribution upper bound");

   return params;
}


UniformDistribution::UniformDistribution(const InputParameters & parameters):
  distribution(parameters), BasicUniformDistribution(getParam<double>("xMin"), getParam<double>("xMax"))
{
}

UniformDistribution::~UniformDistribution()
{
}


/*
 * CLASS NORMAL DISTRIBUTION
 */

template<>
InputParameters validParams<NormalDistribution>(){

   InputParameters params = validParams<distribution>();

   params.addRequiredParam<double>("mu", "Mean");
   params.addRequiredParam<double>("sigma", "Standard deviation");
   return params;
}

NormalDistribution::NormalDistribution(const InputParameters & parameters):
  distribution(parameters),
  BasicNormalDistribution(getParam<double>("mu"),getParam<double>("sigma")) {
}

NormalDistribution::~NormalDistribution(){
}


/*
 * CLASS LOG NORMAL DISTRIBUTION
 */

template<>
InputParameters validParams<LogNormalDistribution>(){

   InputParameters params = validParams<distribution>();

   params.addRequiredParam<double>("mu", "Mean");
   params.addRequiredParam<double>("sigma", "Standard deviation");
   params.addParam<double>("low",0.0, "low");

   return params;
}

LogNormalDistribution::LogNormalDistribution(const InputParameters & parameters):
  distribution(parameters), BasicLogNormalDistribution(getParam<double>("mu"), getParam<double>("sigma"),getParam<double>("low"))
{
}

LogNormalDistribution::~LogNormalDistribution()
{
}

/*
 * CLASS LOGISTIC DISTRIBUTION
 */

template<>
InputParameters validParams<LogisticDistribution>(){

   InputParameters params = validParams<distribution>();

   params.addRequiredParam<double>("scale", "Scale");
   params.addRequiredParam<double>("location", "Location");

   return params;
}

LogisticDistribution::LogisticDistribution(const InputParameters & parameters):
  distribution(parameters), BasicLogisticDistribution(getParam<double>("location"), getParam<double>("scale"))
{
}

LogisticDistribution::~LogisticDistribution()
{
}

/*
 * CLASS TRIANGULAR DISTRIBUTION
 */

template<>
InputParameters validParams<TriangularDistribution>(){

   InputParameters params = validParams<distribution>();

   params.addRequiredParam<double>("xPeak", "Maximum coordinate");
   params.addRequiredParam<double>("lowerBound", "Lower bound");
   params.addRequiredParam<double>("upperBound", "Upper bound");
   return params;
}

TriangularDistribution::TriangularDistribution(const InputParameters & parameters):
  distribution(parameters), BasicTriangularDistribution(getParam<double>("xPeak"),getParam<double>("lowerBound"),getParam<double>("upperBound"))
{
}
TriangularDistribution::~TriangularDistribution()
{
}


/*
 * CLASS EXPONENTIAL DISTRIBUTION
 */

template<>
InputParameters validParams<ExponentialDistribution>(){

   InputParameters params = validParams<distribution>();

   params.addRequiredParam<double>("lambda", "lambda");
   params.addParam<double>("low",0.0, "low");
   return params;
}

ExponentialDistribution::ExponentialDistribution(const InputParameters & parameters):
  distribution(parameters), BasicExponentialDistribution(getParam<double>("lambda"),getParam<double>("low"))
{
}
ExponentialDistribution::~ExponentialDistribution()
{
}

/*
 * CLASS WEIBULL DISTRIBUTION
 */

template<>
InputParameters validParams<WeibullDistribution>(){

   InputParameters params = validParams<distribution>();

   params.addRequiredParam<double>("k", "shape parameter");
   params.addRequiredParam<double>("lambda", "scale parameter");
   params.addParam<double>("low",0.0, "low");
   return params;
}

WeibullDistribution::WeibullDistribution(const InputParameters & parameters):
  distribution(parameters),
  BasicWeibullDistribution(getParam<double>("k"),getParam<double>("lambda"),getParam<double>("low"))

{
}

WeibullDistribution::~WeibullDistribution()
{
}

/*
 * CLASS GAMMA DISTRIBUTION
 */

template<>
InputParameters validParams<GammaDistribution>(){

   InputParameters params = validParams<distribution>();

   params.addRequiredParam<double>("k", "shape parameter");
   params.addRequiredParam<double>("theta", "scale parameter");
   params.addParam<double>("low",0.0,"low value for distribution");
   return params;
}

GammaDistribution::GammaDistribution(const InputParameters & parameters):
  distribution(parameters),
  BasicGammaDistribution(getParam<double>("k"),getParam<double>("theta"),getParam<double>("low"))

{
}

GammaDistribution::~GammaDistribution()
{
}

/*
 * CLASS BETA DISTRIBUTION
 */

template<>
InputParameters validParams<BetaDistribution>(){

   InputParameters params = validParams<distribution>();

   params.addRequiredParam<double>("alpha", "alpha parameter");
   params.addRequiredParam<double>("beta", "beta parameter");
   params.addParam<double>("scale",1.0,"scale value for distribution");
   params.addParam<double>("low",0.0,"low value for distribution");
   return params;
}

BetaDistribution::BetaDistribution(const InputParameters & parameters):
  distribution(parameters),
  BasicBetaDistribution(getParam<double>("alpha"),getParam<double>("beta"),
                        getParam<double>("scale"),getParam<double>("low"))

{
}

BetaDistribution::~BetaDistribution()
{
}

/*
 * CLASS POISSON DISTRIBUTION
 */

template<>
InputParameters validParams<PoissonDistribution>(){

   InputParameters params = validParams<distribution>();

   params.addRequiredParam<double>("mu", "mu parameter (mean)");
   return params;
}

PoissonDistribution::PoissonDistribution(const InputParameters & parameters):
  distribution(parameters),
  BasicPoissonDistribution(getParam<double>("mu"))

{
}

PoissonDistribution::~PoissonDistribution()
{
}

/*
 * CLASS BINOMIAL DISTRIBUTION
 */

template<>
InputParameters validParams<BinomialDistribution>(){

   InputParameters params = validParams<distribution>();

   params.addRequiredParam<double>("n", "n parameter (number of independent experiments)");
   params.addRequiredParam<double>("p", "p parameter (probability of success with each independent experiment)");
   return params;
}

BinomialDistribution::BinomialDistribution(const InputParameters & parameters):
  distribution(parameters),
  BasicBinomialDistribution(getParam<double>("n"),getParam<double>("p"))

{
}

BinomialDistribution::~BinomialDistribution()
{
}

/*
 * CLASS BERNOULLI DISTRIBUTION
 */

template<>
InputParameters validParams<BernoulliDistribution>(){

   InputParameters params = validParams<distribution>();

   params.addRequiredParam<double>("p", "p parameter (probability of success with each independent experiment)");
   return params;
}

BernoulliDistribution::BernoulliDistribution(const InputParameters & parameters):
  distribution(parameters),
  BasicBernoulliDistribution(getParam<double>("p"))

{
}

BernoulliDistribution::~BernoulliDistribution()
{
}

/*
 * CLASS CONSTANT DISTRIBUTION
 */

template<>
InputParameters validParams<ConstantDistribution>(){

   InputParameters params = validParams<distribution>();

   params.addRequiredParam<double>("value", "the value this distribution always returns");
   return params;
}

ConstantDistribution::ConstantDistribution(const InputParameters & parameters):
  distribution(parameters),
  BasicConstantDistribution(getParam<double>("value"))
{
}

ConstantDistribution::~ConstantDistribution()
{
}

/*
 * CLASS CUSTOM DISTRIBUTION
 */

// template<>
// InputParameters validParams<CustomDistribution>(){

//    InputParameters params = validParams<distribution>();

//    params.addRequiredParam< vector<double> >("x_coordinates", "coordinates along x");
//    params.addRequiredParam< vector<double> >("y_coordinates", "coordinates along y");
//    MooseEnum fitting_enum("step_left=0,step_right=1,linear=2,cubic_spline=3");
//    params.addRequiredParam<MooseEnum>("fitting_type",fitting_enum, "type of fitting");
//    params.addParam<int>("n_points",3,"Number of fitting point (for spline only)");
//    return params;
// }

// class CustomDistribution;

// CustomDistribution::CustomDistribution(const InputParameters & parameters):
//   distribution(parameters),
//   BasicCustomDistribution(getParam<double>("x_coordinates"),
//                           getParam<double>("y_coordinates"),
//                           getParam<MooseEnum>("fitting_type"),
//                           getParam<double>("n_points"))
// {
// }

// CustomDistribution::~CustomDistribution()
// {
// }
