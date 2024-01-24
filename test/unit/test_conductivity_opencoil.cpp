#include "open_coil.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

extern const char * DATA_DIR;

// Conductivity ratio between the two materials
const double r = 3;

double
sigma(const mfem::Vector & x, double t)
{
  return x[1] > 0 ? 1 : r;
}

TEST_CASE("ConductivityOpenCoil", "[CheckData]")
{

  // Floating point error tolerance
  const double eps{1e-10};
  int order = 1;

  mfem::Mesh mesh((std::string(DATA_DIR) + "inhomogeneous_beam.g").c_str(), 1, 1);
  std::shared_ptr<mfem::ParMesh> pmesh =
      std::make_shared<mfem::ParMesh>(mfem::ParMesh(MPI_COMM_WORLD, mesh));

  mfem::ND_FECollection h_curl_collection(order, pmesh.get()->Dimension());
  mfem::ParFiniteElementSpace h_curl_fe_space(pmesh.get(), &h_curl_collection);
  mfem::ParGridFunction e(&h_curl_fe_space);

  double ival = 10.0;
  mfem::ConstantCoefficient itot(ival);
  mfem::FunctionCoefficient conductivity(sigma);

  hephaestus::InputParameters ocs_params;
  hephaestus::BCMap bc_maps;

  hephaestus::Coefficients coefficients;
  coefficients._scalars.Register(std::string("Itotal"), &itot, false);
  coefficients._scalars.Register(std::string("Conductivity"), &conductivity, false);

  hephaestus::FESpaces fespaces;
  fespaces.Register(std::string("HCurl"), &h_curl_fe_space, false);

  hephaestus::GridFunctions gridfunctions;
  gridfunctions.Register(std::string("E"), &e, false);

  ocs_params.SetParam("GradPotentialName", std::string("E"));
  ocs_params.SetParam("IFuncCoefName", std::string("Itotal"));
  ocs_params.SetParam("PotentialName", std::string("V"));
  ocs_params.SetParam("ConductivityCoefName", std::string("Conductivity"));
  ocs_params.SetParam("GradPhiTransfer", true);

  std::pair<int, int> elec_attrs{2, 3};
  mfem::Array<int> submesh_domains;
  submesh_domains.Append(1);

  hephaestus::OpenCoilSolver opencoil(ocs_params, submesh_domains, elec_attrs);
  opencoil.Init(gridfunctions, fespaces, bc_maps, coefficients);
  mfem::ParLinearForm dummy(&h_curl_fe_space);
  opencoil.Apply(&dummy);

  double flux1 = hephaestus::calcFlux(&e, 4, conductivity);
  double flux2 = hephaestus::calcFlux(&e, 5, conductivity);

  REQUIRE_THAT(flux1 + flux2, Catch::Matchers::WithinAbs(ival, eps));
  REQUIRE_THAT(flux1 / flux2, Catch::Matchers::WithinAbs(r, eps));
}