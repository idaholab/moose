#include "time_domain_em_formulation.hpp"

namespace hephaestus
{

// enum EMField {
//   ElectricField,
//   MagneticField,
//   MagneticFluxDensity,
//   CurrentDensity,
//   LorentzForceDensity,
//   JouleHeatingPower
// };

// enum EMPotential {
//   MagneticVectorPotential,
//   ElectricScalarPotential,
//   ElectricVectorPotential,
//   MagneticScalarPotential
// };

// enum EMMaterialProperty {
//   ElectricConductivity,
//   ElectricResistivity,
//   MagneticPermeability,
//   MagneticReluctivity,
//   ElectricPermittivity
// };

// // req_coef = blah
// // check

// Struct: motive: to automate AddAuxSolver
// and more easily validate fields

// Formulation(EMFieldSet, EMMaterialProperties)
// struct EMFieldSet:
// active_fields = std::map<EMField, longname>

// std::string enum_to_longname(EMField field) {
//   switch (field) {
//   case ElectricField:
//     return "electric_field";
//   case MagneticField:
//     return "magnetic_field";
//   case MagneticFluxDensity:
//     return "magnetic_flux_density";
//   case CurrentDensity:
//     return "current_density";
//   default:
//     return "";
//   }
// }

TimeDomainEMFormulation::TimeDomainEMFormulation() = default;
;

} // namespace hephaestus
