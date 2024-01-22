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

#include <utility>

namespace hephaestus
{

EFormulation::EFormulation(const std::string & magnetic_reluctivity_name,
                           std::string magnetic_permeability_name,
                           const std::string & electric_conductivity_name,
                           const std::string & e_field_name)
  : HCurlFormulation(magnetic_reluctivity_name, electric_conductivity_name, e_field_name),
    _magnetic_permeability_name(std::move(magnetic_permeability_name))
{
}

void
EFormulation::RegisterCoefficients()
{
  hephaestus::Coefficients & coefficients = GetProblem()->_coefficients;
  if (!coefficients._scalars.Has(_magnetic_permeability_name))
  {
    MFEM_ABORT(_magnetic_permeability_name + " coefficient not found.");
  }
  if (!coefficients._scalars.Has(_electric_conductivity_name))
  {
    MFEM_ABORT(_electric_conductivity_name + " coefficient not found.");
  }
  coefficients._scalars.Register(
      _magnetic_reluctivity_name,
      new mfem::TransformedCoefficient(
          &_one_coef, coefficients._scalars.Get(_magnetic_permeability_name), fracFunc),
      true);
}

void
EFormulation::RegisterCurrentDensityAux(const std::string & j_field_name)
{
  //* Current density J = Jᵉ + σE
  //* Induced electric field, Jind = σE
  hephaestus::AuxSolvers & auxsolvers = GetProblem()->_postprocessors;
  auxsolvers.Register(j_field_name,
                      new hephaestus::ScaledVectorGridFunctionAux(
                          _h_curl_var_name, j_field_name, _electric_conductivity_name),
                      true);
}

void
EFormulation::RegisterJouleHeatingDensityAux(const std::string & p_field_name,
                                             const std::string & e_field_name,
                                             const std::string & conductivity_coef_name)
{
  //* Joule heating density = E.J
  hephaestus::AuxSolvers & auxsolvers = GetProblem()->_postprocessors;
  auxsolvers.Register(
      p_field_name,
      new hephaestus::VectorGridFunctionDotProductAux(
          p_field_name, p_field_name, _electric_conductivity_name, e_field_name, e_field_name),
      true);
  auxsolvers.Get(p_field_name)->SetPriority(2);
}

} // namespace hephaestus
