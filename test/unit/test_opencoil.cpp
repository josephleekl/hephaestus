#include "open_coil.hpp"
#include <gtest/gtest.h>

extern const char *DATA_DIR;

TEST(OpenCoilTest, CheckData) {

  int par_ref_lvl = -1;
  int order = 1;

  mfem::Mesh mesh((std::string(DATA_DIR) + "coil.gen").c_str(), 1, 1);
  std::shared_ptr<mfem::ParMesh> pmesh =
      std::make_shared<mfem::ParMesh>(mfem::ParMesh(MPI_COMM_WORLD, mesh));

  for (int l = 0; l < par_ref_lvl; ++l)
    pmesh->UniformRefinement();

  mfem::ND_FECollection HCurl_Collection(order, pmesh.get()->Dimension());
  mfem::ParFiniteElementSpace HCurlFESpace(pmesh.get(), &HCurl_Collection);
  mfem::ParGridFunction j(&HCurlFESpace);

  double Ival = 10.0;
  double cond_val = 1e6;
  mfem::ConstantCoefficient Itot(Ival);
  mfem::ConstantCoefficient Conductivity(cond_val);

  hephaestus::InputParameters ocs_params;
  hephaestus::BCMap bc_maps;

  hephaestus::Coefficients coefficients;
  coefficients.scalars.Register(std::string("Itotal"), &Itot, true);
  coefficients.scalars.Register(std::string("Conductivity"), &Conductivity,
                                true);

  hephaestus::FESpaces fespaces;
  fespaces.Register(std::string("HCurl"), &HCurlFESpace, true);

  hephaestus::GridFunctions gridfunctions;
  gridfunctions.Register(std::string("J"), &j, true);

  ocs_params.SetParam("SourceName", std::string("J"));
  ocs_params.SetParam("IFuncCoefName", std::string("Itotal"));
  ocs_params.SetParam("PotentialName", std::string("V"));
  ocs_params.SetParam("ConductivityCoefName", std::string("Conductivity"));

  std::pair<int, int> elec_attrs{1, 2};
  mfem::Array<int> submesh_domains;
  submesh_domains.Append(1);

  hephaestus::OpenCoilSolver opencoil(ocs_params, submesh_domains, elec_attrs);
  opencoil.Init(gridfunctions, fespaces, bc_maps, coefficients);
  mfem::ParLinearForm dummy(&HCurlFESpace);
  opencoil.Apply(&dummy);

  double flux = hephaestus::calcFlux(&j, elec_attrs.first, Conductivity);

  EXPECT_FLOAT_EQ(flux, Ival);
}