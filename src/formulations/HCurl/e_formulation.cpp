//* Solves:
//* ∇×(ν∇×E) + σdE/dt = -dJᵉ/dt
//*
//* in weak form
//* (ν∇×E, ∇×E') + (σdE/dt, E') + (dJᵉ/dt, E') - <(ν∇×E)×n, E'>  = 0

//* where:
//* reluctivity ν = 1/μ
//* electrical_conductivity σ=1/ρ
//* Electric Field E
//* Current density J = σE
//* Magnetic flux density, dB/dt = -∇×E
//* Magnetic field dH/dt = -ν∇×E

#include "e_formulation.hpp"

namespace hephaestus {

EFormulation::EFormulation() : HCurlFormulation() {
  alpha_coef_name = std::string("magnetic_reluctivity");
  beta_coef_name = std::string("electrical_conductivity");
  h_curl_var_name = std::string("electric_field");
}

void EFormulation::RegisterCoefficients() {
  hephaestus::Coefficients &coefficients = this->GetProblem()->coefficients;
  if (!coefficients.scalars.Has("magnetic_permeability")) {
    MFEM_ABORT("magnetic_permeability coefficient not found.");
  }
  if (!coefficients.scalars.Has("electrical_conductivity")) {
    MFEM_ABORT("electrical_conductivity coefficient not found.");
  }
  coefficients.scalars.Register(
      alpha_coef_name,
      new mfem::TransformedCoefficient(
          &oneCoef, coefficients.scalars.Get("magnetic_permeability"),
          fracFunc),
      true);
}

} // namespace hephaestus