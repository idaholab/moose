//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVPhaseChangeEnthalpyFunctorMaterial.h"

#include "PhaseChangeEnthalpyUtils.h"

#include "Function.h"

registerMooseObject("NavierStokesApp", INSFVPhaseChangeEnthalpyFunctorMaterial);

InputParameters
INSFVPhaseChangeEnthalpyFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription(
      "Defines a piecewise enthalpy-temperature relationship for phase-change problems and "
      "provides h_from_p_T, T_from_p_h, temperature, liquid_fraction, and dTdh functors. "
      "Optionally integrates temperature-dependent cp(T) correlations over temperature.");

  params.addRequiredParam<MooseFunctorName>("cp_solid", "Solid specific heat capacity.");
  params.addRequiredParam<MooseFunctorName>("cp_liquid", "Liquid specific heat capacity.");
  params.addRequiredParam<MooseFunctorName>("L", "Latent heat of fusion.");
  params.addRequiredParam<MooseFunctorName>("T_solidus", "Solidus temperature.");
  params.addRequiredParam<MooseFunctorName>("T_liquidus", "Liquidus temperature.");

  params.addRequiredParam<MooseFunctorName>(
      "temperature",
      "Temperature functor used to compute h_from_p_T (typically for boundary conditions).");
  params.addRequiredParam<MooseFunctorName>(
      "enthalpy", "Specific enthalpy functor used to compute T_from_p_h.");

  params.addParam<MooseFunctorName>(
      "h_from_p_T_name", "h_from_p_T", "Name of the enthalpy-from-T functor.");
  params.addParam<MooseFunctorName>(
      "T_from_p_h_name", "T_from_p_h", "Name of the T-from-enthalpy functor.");
  params.addParam<MooseFunctorName>(
      "temperature_name",
      "temperature",
      "Name of the temperature functor (alias of T_from_p_h)." );
  params.addParam<MooseFunctorName>(
      "liquid_fraction_name", "liquid_fraction", "Name of the liquid fraction functor." );
  params.addParam<MooseFunctorName>(
      "dTdh_name",
      "dTdh",
      "Name of the dT/dh functor (temperature derivative w.r.t. enthalpy)." );

  // Optional: temperature-dependent cp(T) integration.
  params.addParam<bool>(
      "integrate_cp_over_temperature",
      false,
      "If true, compute sensible enthalpy in the solid and liquid ranges as integrals of cp(T) "
      "over temperature using cp_solid_T_function and cp_liquid_T_function. The functions are "
      "evaluated with x=temperature.");

  params.addParam<FunctionName>(
      "cp_solid_T_function",
      "Optional solid cp(T) function, evaluated as f(time, Point(T,0,0)). Required when "
      "integrate_cp_over_temperature=true." );
  params.addParam<FunctionName>(
      "cp_liquid_T_function",
      "Optional liquid cp(T) function, evaluated as f(time, Point(T,0,0)). Required when "
      "integrate_cp_over_temperature=true." );

  params.addParam<unsigned int>(
      "cp_integration_subintervals",
      64,
      "Number of subintervals for composite Simpson integration of cp(T). An even number is "
      "enforced." );

  params.addParam<unsigned int>(
      "cp_inversion_max_its", 60, "Maximum iterations for inverting h(T) via bisection." );
  params.addParam<Real>(
      "cp_inversion_rel_tol",
      1e-10,
      "Relative tolerance for inverting h(T) (scaled by |h_target|)." );
  params.addParam<Real>("cp_inversion_abs_tol", 1e-12, "Absolute tolerance for inverting h(T)." );

  return params;
}

INSFVPhaseChangeEnthalpyFunctorMaterial::INSFVPhaseChangeEnthalpyFunctorMaterial(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _cp_s(getFunctor<Real>("cp_solid")),
    _cp_l(getFunctor<Real>("cp_liquid")),
    _L(getFunctor<Real>("L")),
    _T_solidus(getFunctor<Real>("T_solidus")),
    _T_liquidus(getFunctor<Real>("T_liquidus")),
    _temperature_in(getFunctor<Real>("temperature")),
    _enthalpy_in(getFunctor<Real>("enthalpy")),
    _integrate_cp_over_T(getParam<bool>("integrate_cp_over_temperature")),
    _cp_solid_T_function(nullptr),
    _cp_liquid_T_function(nullptr),
    _cp_integration_subintervals(getParam<unsigned int>("cp_integration_subintervals")),
    _cp_inversion_max_its(getParam<unsigned int>("cp_inversion_max_its")),
    _cp_inversion_rel_tol(getParam<Real>("cp_inversion_rel_tol")),
    _cp_inversion_abs_tol(getParam<Real>("cp_inversion_abs_tol"))
{
  if (_integrate_cp_over_T)
  {
    if (!isParamValid("cp_solid_T_function") || !isParamValid("cp_liquid_T_function"))
      paramError(
          "integrate_cp_over_temperature",
          "When integrate_cp_over_temperature=true you must provide both cp_solid_T_function and "
          "cp_liquid_T_function." );

    _cp_solid_T_function = &getFunction("cp_solid_T_function");
    _cp_liquid_T_function = &getFunction("cp_liquid_T_function");

    if (_cp_integration_subintervals < 2)
      paramError("cp_integration_subintervals", "Must be >= 2." );
    if (_cp_inversion_max_its < 1)
      paramError("cp_inversion_max_its", "Must be >= 1." );
  }

  const auto h_from_p_T_name = getParam<MooseFunctorName>("h_from_p_T_name");
  const auto T_from_p_h_name = getParam<MooseFunctorName>("T_from_p_h_name");
  const auto temperature_name = getParam<MooseFunctorName>("temperature_name");
  const auto liquid_fraction_name = getParam<MooseFunctorName>("liquid_fraction_name");
  const auto dTdh_name = getParam<MooseFunctorName>("dTdh_name");

  // ------------------------------------------------------------
  // h(T): specific enthalpy from the provided temperature functor.
  // Reference: h(T_solidus) = 0.
  // ------------------------------------------------------------
//   addFunctorProperty<Real>(
//       h_from_p_T_name,
//       [this](const auto & r, const auto & state) -> Real
//       {
//         const Real time = PhaseChangeEnthalpyUtils::timeFromStateArg(state);

//         const Real T_sol = _T_solidus(r, state);
//         const Real T_liq = _T_liquidus(r, state);
//         const Real L = _L(r, state);
//         const Real T = _temperature_in(r, state);

//         const Real dT_pc = T_liq - T_sol;

//         // Degenerate mushy interval: sharp jump at T_solidus
//         if (dT_pc <= 0.0)
//         {
//           if (T < T_sol)
//           {
//             if (_integrate_cp_over_T)
//               return PhaseChangeEnthalpyUtils::integrateTemperatureFunctionSimpson(
//                   *_cp_solid_T_function, time, T_sol, T, _cp_integration_subintervals);
//             else
//               return _cp_s(r, state) * (T - T_sol);
//           }
//           else
//           {
//             if (_integrate_cp_over_T)
//               return L + PhaseChangeEnthalpyUtils::integrateTemperatureFunctionSimpson(
//                              *_cp_liquid_T_function, time, T_sol, T, _cp_integration_subintervals);
//             else
//               return L + _cp_l(r, state) * (T - T_sol);
//           }
//         }

//         // Non-degenerate mushy interval
//         if (T <= T_sol)
//         {
//           if (_integrate_cp_over_T)
//             return PhaseChangeEnthalpyUtils::integrateTemperatureFunctionSimpson(
//                 *_cp_solid_T_function, time, T_sol, T, _cp_integration_subintervals);
//           else
//             return _cp_s(r, state) * (T - T_sol);
//         }
//         else if (T >= T_liq)
//         {
//           if (_integrate_cp_over_T)
//             return L + PhaseChangeEnthalpyUtils::integrateTemperatureFunctionSimpson(
//                            *_cp_liquid_T_function, time, T_liq, T, _cp_integration_subintervals);
//           else
//             return L + _cp_l(r, state) * (T - T_liq);
//         }
//         else
//         {
//           // Mushy: latent only, linear liquid fraction
//           return (L > 0.0) ? L * (T - T_sol) / dT_pc : 0.0;
//         }
//       });

//   // ------------------------------------------------------------
//   // T(h): temperature from the provided enthalpy functor.
//   // ------------------------------------------------------------
//   const auto T_from_h_eval = [this](const auto & r, const auto & state) -> Real
//   {
//     const Real time = PhaseChangeEnthalpyUtils::timeFromStateArg(state);

//     const Real T_sol = _T_solidus(r, state);
//     const Real T_liq = _T_liquidus(r, state);
//     const Real L = _L(r, state);
//     const Real h = _enthalpy_in(r, state);

//     const Real dT_pc = T_liq - T_sol;

//     // Degenerate mushy interval: sharp jump at T_solidus
//     if (dT_pc <= 0.0)
//     {
//       if (h < 0.0)
//       {
//         if (_integrate_cp_over_T)
//           return PhaseChangeEnthalpyUtils::invertIntegratedCpBisection(
//               *_cp_solid_T_function,
//               time,
//               T_sol,
//               h,
//               _cp_integration_subintervals,
//               _cp_inversion_max_its,
//               _cp_inversion_rel_tol,
//               _cp_inversion_abs_tol);
//         else
//         {
//           const Real cp = _cp_s(r, state);
//           return (cp != 0.0) ? (T_sol + h / cp) : T_sol;
//         }
//       }
//       else if (h > L)
//       {
//         if (_integrate_cp_over_T)
//           return PhaseChangeEnthalpyUtils::invertIntegratedCpBisection(
//               *_cp_liquid_T_function,
//               time,
//               T_sol,
//               h - L,
//               _cp_integration_subintervals,
//               _cp_inversion_max_its,
//               _cp_inversion_rel_tol,
//               _cp_inversion_abs_tol);
//         else
//         {
//           const Real cp = _cp_l(r, state);
//           return (cp != 0.0) ? (T_sol + (h - L) / cp) : T_sol;
//         }
//       }
//       else
//         // Any h in [0, L] maps to T = T_solidus for a sharp interface
//         return T_sol;
//     }

//     // Non-degenerate mushy interval
//     if (h <= 0.0)
//     {
//       if (_integrate_cp_over_T)
//         return PhaseChangeEnthalpyUtils::invertIntegratedCpBisection(
//             *_cp_solid_T_function,
//             time,
//             T_sol,
//             h,
//             _cp_integration_subintervals,
//             _cp_inversion_max_its,
//             _cp_inversion_rel_tol,
//             _cp_inversion_abs_tol);
//       else
//       {
//         const Real cp = _cp_s(r, state);
//         return (cp != 0.0) ? (T_sol + h / cp) : T_sol;
//       }
//     }
//     else if (h >= L)
//     {
//       if (_integrate_cp_over_T)
//         return PhaseChangeEnthalpyUtils::invertIntegratedCpBisection(
//             *_cp_liquid_T_function,
//             time,
//             T_liq,
//             h - L,
//             _cp_integration_subintervals,
//             _cp_inversion_max_its,
//             _cp_inversion_rel_tol,
//             _cp_inversion_abs_tol);
//       else
//       {
//         const Real cp = _cp_l(r, state);
//         return (cp != 0.0) ? (T_liq + (h - L) / cp) : T_liq;
//       }
//     }
//     else
//     {
//       // Mushy: invert h = L * (T - T_sol) / (T_liq - T_sol)
//       return (L > 0.0) ? (T_sol + dT_pc * (h / L)) : T_sol;
//     }
//   };

//   addFunctorProperty<Real>(T_from_p_h_name, T_from_h_eval);

//   // Provide an explicit temperature functor (alias of T_from_p_h)
//   addFunctorProperty<Real>(temperature_name, T_from_h_eval);

//   // ------------------------------------------------------------
//   // Liquid fraction: f_l = clamp(h/L, 0, 1)
//   // ------------------------------------------------------------
//   addFunctorProperty<Real>(
//       liquid_fraction_name,
//       [this](const auto & r, const auto & state) -> Real
//       {
//         const Real L = _L(r, state);
//         if (L <= 0.0)
//           return 0.0;

//         const Real h = _enthalpy_in(r, state);
//         if (h <= 0.0)
//           return 0.0;
//         if (h >= L)
//           return 1.0;
//         return h / L;
//       });

//   // ------------------------------------------------------------
//   // dT/dh: used to rewrite k*grad(T) as (k*dTdh)*grad(h)
//   // ------------------------------------------------------------
//   addFunctorProperty<Real>(
//       dTdh_name,
//       [this, T_from_h_eval](const auto & r, const auto & state) -> Real
//       {
//         const Real time = PhaseChangeEnthalpyUtils::timeFromStateArg(state);

//         const Real T_sol = _T_solidus(r, state);
//         const Real T_liq = _T_liquidus(r, state);
//         const Real L = _L(r, state);
//         const Real h = _enthalpy_in(r, state);

//         const Real dT_pc = T_liq - T_sol;

//         // Degenerate mushy interval: sharp interface
//         if (dT_pc <= 0.0)
//         {
//           if (h < 0.0)
//           {
//             if (_integrate_cp_over_T)
//             {
//               const Real T = T_from_h_eval(r, state);
//               const Real cp = PhaseChangeEnthalpyUtils::evalTemperatureFunction(
//                   *_cp_solid_T_function, time, T);
//               return (cp > 0.0) ? 1.0 / cp : 0.0;
//             }
//             else
//             {
//               const Real cp = _cp_s(r, state);
//               return (cp != 0.0) ? 1.0 / cp : 0.0;
//             }
//           }
//           else if (h > L)
//           {
//             if (_integrate_cp_over_T)
//             {
//               const Real T = T_from_h_eval(r, state);
//               const Real cp = PhaseChangeEnthalpyUtils::evalTemperatureFunction(
//                   *_cp_liquid_T_function, time, T);
//               return (cp > 0.0) ? 1.0 / cp : 0.0;
//             }
//             else
//             {
//               const Real cp = _cp_l(r, state);
//               return (cp != 0.0) ? 1.0 / cp : 0.0;
//             }
//           }
//           else
//             return 0.0;
//         }

//         // Non-degenerate mushy interval
//         if (h <= 0.0)
//         {
//           if (_integrate_cp_over_T)
//           {
//             const Real T = T_from_h_eval(r, state);
//             const Real cp = PhaseChangeEnthalpyUtils::evalTemperatureFunction(
//                 *_cp_solid_T_function, time, T);
//             return (cp > 0.0) ? 1.0 / cp : 0.0;
//           }
//           else
//           {
//             const Real cp = _cp_s(r, state);
//             return (cp != 0.0) ? 1.0 / cp : 0.0;
//           }
//         }
//         else if (h >= L)
//         {
//           if (_integrate_cp_over_T)
//           {
//             const Real T = T_from_h_eval(r, state);
//             const Real cp = PhaseChangeEnthalpyUtils::evalTemperatureFunction(
//                 *_cp_liquid_T_function, time, T);
//             return (cp > 0.0) ? 1.0 / cp : 0.0;
//           }
//           else
//           {
//             const Real cp = _cp_l(r, state);
//             return (cp != 0.0) ? 1.0 / cp : 0.0;
//           }
//         }
//         else
//           return (L > 0.0) ? dT_pc / L : 0.0;
//       });
// }

const auto clamp01 = [](const Real x) -> Real
{
  return std::max(0.0, std::min(1.0, x));
};

const auto smoothstep01 = [clamp01](const Real x) -> Real
{
  const Real xi = clamp01(x);
  return xi * xi * (3.0 - 2.0 * xi); // 3 xi^2 - 2 xi^3
};

const auto dsmoothstep01 = [clamp01](const Real x) -> Real
{
  const Real xi = clamp01(x);
  return 6.0 * xi * (1.0 - xi); // d/dxi of smoothstep
};

// ------------------------------------------------------------
// H(T): total enthalpy from temperature
//
// Reference: H(T_sol) = 0
//
// In the mushy interval:
//   xi      = (T - T_sol)/dT_pc
//   f(xi)   = 3 xi^2 - 2 xi^3
//   cp_mix  = (1 - f) cp_s + f cp_l
//   H(T)    = integral(cp_mix dT) + L f
//
// For constant cp_s and cp_l, this becomes
//   H(T) = cp_s dT_pc xi
//        + (cp_l - cp_s) dT_pc (xi^3 - 0.5 xi^4)
//        + L (3 xi^2 - 2 xi^3)
// ------------------------------------------------------------
const auto h_from_T_eval = [this, smoothstep01](const auto & r,
                                                const auto & state,
                                                const Real T) -> Real
{
  const Real T_sol = _T_solidus(r, state);
  const Real T_liq = _T_liquidus(r, state);
  const Real L = _L(r, state);

  const Real cp_s = _cp_s(r, state);
  const Real cp_l = _cp_l(r, state);

  const Real dT_pc = T_liq - T_sol;

  // Degenerate mushy interval: sharp jump at T_solidus
  if (dT_pc <= 0.0)
  {
    if (T < T_sol)
      return cp_s * (T - T_sol);
    else
      return L + cp_l * (T - T_sol);
  }

  // NOTE:
  // Even with smoothing, H(T_liq) is still:
  //   h_liq = 0.5 * (cp_s + cp_l) * dT_pc + L
  // because average(smoothstep) on [0,1] is 1/2.
  const Real h_liq = 0.5 * (cp_s + cp_l) * dT_pc + L;

  if (T <= T_sol)
    return cp_s * (T - T_sol);
  else if (T >= T_liq)
    return h_liq + cp_l * (T - T_liq);
  else
  {
    const Real xi = (T - T_sol) / dT_pc;
    const Real xi2 = xi * xi;
    const Real xi3 = xi2 * xi;
    const Real xi4 = xi2 * xi2;

    const Real dcp = cp_l - cp_s;
    const Real f = smoothstep01(xi);

    return cp_s * dT_pc * xi
         + dcp * dT_pc * (xi3 - 0.5 * xi4)
         + L * f;
  }
};

addFunctorProperty<Real>(
    h_from_p_T_name,
    [this, h_from_T_eval](const auto & r, const auto & state) -> Real
    {
      const Real T = _temperature_in(r, state);
      return h_from_T_eval(r, state, T);
    });

// ------------------------------------------------------------
// T(H): temperature from total enthalpy
//
// NOTE:
// With smoothing, the mushy H(T) is no longer quadratic.
// So the old closed-form inversion is not valid.
// We invert in the mushy interval with bisection.
// ------------------------------------------------------------
const auto T_from_h_eval = [this, h_from_T_eval](const auto & r, const auto & state) -> Real
{
  const Real T_sol = _T_solidus(r, state);
  const Real T_liq = _T_liquidus(r, state);
  const Real L = _L(r, state);
  const Real h = _enthalpy_in(r, state);

  const Real cp_s = _cp_s(r, state);
  const Real cp_l = _cp_l(r, state);

  const Real dT_pc = T_liq - T_sol;

  // Degenerate mushy interval: sharp jump at T_solidus
  if (dT_pc <= 0.0)
  {
    if (h < 0.0)
      return (cp_s != 0.0) ? (T_sol + h / cp_s) : T_sol;
    else if (h > L)
      return (cp_l != 0.0) ? (T_sol + (h - L) / cp_l) : T_sol;
    else
      return T_sol;
  }

  const Real h_liq = 0.5 * (cp_s + cp_l) * dT_pc + L;

  if (h <= 0.0)
    return (cp_s != 0.0) ? (T_sol + h / cp_s) : T_sol;
  else if (h >= h_liq)
    return (cp_l != 0.0) ? (T_liq + (h - h_liq) / cp_l) : T_liq;
  else
  {
    // Mushy interval inversion by bisection on [T_sol, T_liq]
    const Real tol = std::max(_cp_inversion_abs_tol,
                              _cp_inversion_rel_tol * std::max(std::abs(h), std::abs(h_liq)));

    Real T_low = T_sol;
    Real T_high = T_liq;

    for (unsigned int it = 0; it < _cp_inversion_max_its; ++it)
    {
      const Real T_mid = 0.5 * (T_low + T_high);
      const Real h_mid = h_from_T_eval(r, state, T_mid);

      if (std::abs(h_mid - h) <= tol)
        return T_mid;

      if (h_mid < h)
        T_low = T_mid;
      else
        T_high = T_mid;
    }

    return 0.5 * (T_low + T_high);
  }
};

addFunctorProperty<Real>(T_from_p_h_name, T_from_h_eval);
addFunctorProperty<Real>(temperature_name, T_from_h_eval);

// ------------------------------------------------------------
// Liquid fraction
//
// NOTE:
// For non-degenerate phase change, this is now the smoothed f(T),
// not the old linear xi and not h/L.
// ------------------------------------------------------------
addFunctorProperty<Real>(
    liquid_fraction_name,
    [this, T_from_h_eval, clamp01, smoothstep01](const auto & r, const auto & state) -> Real
    {
      const Real T_sol = _T_solidus(r, state);
      const Real T_liq = _T_liquidus(r, state);
      const Real L = _L(r, state);
      const Real h = _enthalpy_in(r, state);

      if (L <= 0.0)
        return 0.0;

      const Real dT_pc = T_liq - T_sol;

      // Sharp interface: keep old convention
      if (dT_pc <= 0.0)
        return clamp01(h / L);

      const Real T = T_from_h_eval(r, state);
      const Real xi = (T - T_sol) / dT_pc;
      return smoothstep01(xi);
    });

// ------------------------------------------------------------
// dT/dH: used to rewrite k*grad(T) as (k*dTdh)*grad(H)
//
// NOTE:
// In mushy region:
//   cp_app = cp_mix + L * df/dT
//   cp_mix = (1 - f) cp_s + f cp_l
//   df/dT  = (df/dxi) / dT_pc
// ------------------------------------------------------------
addFunctorProperty<Real>(
    dTdh_name,
    [this, T_from_h_eval, smoothstep01, dsmoothstep01](const auto & r, const auto & state) -> Real
    {
      const Real T_sol = _T_solidus(r, state);
      const Real T_liq = _T_liquidus(r, state);
      const Real L = _L(r, state);
      const Real h = _enthalpy_in(r, state);

      const Real cp_s = _cp_s(r, state);
      const Real cp_l = _cp_l(r, state);

      const Real dT_pc = T_liq - T_sol;

      // Degenerate mushy interval: sharp interface
      if (dT_pc <= 0.0)
      {
        if (h < 0.0)
          return (cp_s > 0.0) ? 1.0 / cp_s : 0.0;
        else if (h > L)
          return (cp_l > 0.0) ? 1.0 / cp_l : 0.0;
        else
          return 0.0;
      }

      const Real T = T_from_h_eval(r, state);

      if (T <= T_sol)
        return (cp_s > 0.0) ? 1.0 / cp_s : 0.0;
      else if (T >= T_liq)
        return (cp_l > 0.0) ? 1.0 / cp_l : 0.0;
      else
      {
        const Real xi = (T - T_sol) / dT_pc;
        const Real f = smoothstep01(xi);
        const Real df_dT = dsmoothstep01(xi) / dT_pc;

        const Real cp_mix = (1.0 - f) * cp_s + f * cp_l;
        const Real cp_app = cp_mix + L * df_dT;

        return (cp_app > 0.0) ? 1.0 / cp_app : 0.0;
      }
    });
  }
