#ifndef MACRO_G4TPC_C
#define MACRO_G4TPC_C

#include <GlobalVariables.C>
#include <QA.C>

#include <G4_Intt.C>
#include <G4_Mvtx.C>

#include <g4tpc/PHG4TpcDigitizer.h>
#include <g4tpc/PHG4TpcDirectLaser.h>
#include <g4tpc/PHG4TpcDistortion.h>
#include <g4tpc/PHG4TpcElectronDrift.h>
#include <g4tpc/PHG4TpcEndCapSubsystem.h>
#include <g4tpc/PHG4TpcPadPlane.h>
#include <g4tpc/PHG4TpcPadPlaneReadout.h>
#include <g4tpc/PHG4TpcSubsystem.h>

#include <g4main/PHG4Reco.h>

#include <tpc/TpcClusterizer.h>
#include <tpc/TpcClusterCleaner.h>
#include <tpc/TpcSpaceChargeCorrection.h>
#include <qa_modules/QAG4SimulationTpc.h>

#include <fun4all/Fun4AllServer.h>

R__LOAD_LIBRARY(libg4tpc.so)
R__LOAD_LIBRARY(libtpc.so)
R__LOAD_LIBRARY(libqa_modules.so)

namespace Enable
{
  //these are set in the Fun4All macro, just placeholders here.
  bool TPC = false;
  bool TPC_ABSORBER = false;
  bool TPC_OVERLAPCHECK = false;
  bool TPC_CELL = false;
  bool TPC_CLUSTER = false;
  bool TPC_QA = false;

  bool TPC_ENDCAP = true;

  int TPC_VERBOSITY = 1;
}  // namespace Enable

namespace G4TPC
{
  int n_tpc_layer_inner = 16;
  int tpc_layer_rphi_count_inner = 1152;
  int n_tpc_layer_mid = 16;
  int n_tpc_layer_outer = 16;
  int n_gas_layer = n_tpc_layer_inner + n_tpc_layer_mid + n_tpc_layer_outer;
  double tpc_outer_radius = 77. + 2.;

  // distortions
  bool ENABLE_STATIC_DISTORTIONS = true;
  auto static_distortion_filename = std::string(getenv("CALIBRATIONROOT")) + "/TPC/DistortionMaps/fluct_average.rev3.1side.3d.file0.h_negz.real_B1.4_E-400.0.ross_phi1_sphenix_phislice_lookup_r26xp40xz40.distortion_map.hist.root";

  bool ENABLE_TIME_ORDERED_DISTORTIONS = false;
  std::string time_ordered_distortion_filename = "/gpfs/mnt/gpfs02/sphenix/user/klest/TimeOrderedDistortions.root";

//   unsigned int distortion_coordinates =
//     PHG4TpcElectronDrift::COORD_PHI|
//     PHG4TpcElectronDrift::COORD_R|
//     PHG4TpcElectronDrift::COORD_Z;

  // distortion corrections
  bool ENABLE_CORRECTIONS = false;
  auto correction_filename = std::string(getenv("CALIBRATIONROOT")) + "/TPC/DistortionMaps/fluct_average.rev3.1side.3d.file0.h_negz.real_B1.4_E-400.0.ross_phi1_sphenix_phislice_lookup_r26xp40xz40.distortion_map.hist.root";
  unsigned int correction_coordinates =
    TpcSpaceChargeCorrection::COORD_PHI|
    TpcSpaceChargeCorrection::COORD_R|
    TpcSpaceChargeCorrection::COORD_Z;

}  // namespace G4TPC

void TPCInit()
{
  printf(">>>>>>>RCC TPCInit.  So I am using this file for sure.\n");

  
  BlackHoleGeometry::max_radius = std::max(BlackHoleGeometry::max_radius, G4TPC::tpc_outer_radius);

  if (Enable::TPC_ENDCAP)
  {
    BlackHoleGeometry::max_z = std::max(BlackHoleGeometry::max_z, 130.);
    BlackHoleGeometry::min_z = std::min(BlackHoleGeometry::min_z, -130.);
  }
  else
  {
    BlackHoleGeometry::max_z = std::max(BlackHoleGeometry::max_z, 211. / 2.);
    BlackHoleGeometry::min_z = std::min(BlackHoleGeometry::min_z, -211. / 2.);
  }

  // the mvtx is not called if disabled but the default number of layers is set to 3,
  // so we need to set it to zero
  if (!Enable::MVTX)
  {
    G4MVTX::n_maps_layer = 0;
  }
  // same for the INTT
  if (!Enable::INTT)
  {
    G4INTT::n_intt_layer = 0;
  }
}

//! TPC end cap, wagon wheel, electronics
void TPC_Endcaps(PHG4Reco* g4Reco)
{
  bool OverlapCheck = Enable::OVERLAPCHECK || Enable::TPC_OVERLAPCHECK;
  bool AbsorberActive = Enable::ABSORBER || Enable::TPC_ABSORBER;

  PHG4TpcEndCapSubsystem* tpc_endcap = new PHG4TpcEndCapSubsystem("TPC_ENDCAP");
  tpc_endcap->SuperDetector("TPC_ENDCAP");

  if (AbsorberActive) tpc_endcap->SetActive();
  tpc_endcap->OverlapCheck(OverlapCheck);

  //  tpc_endcap->set_int_param("construction_verbosity", 2);

  g4Reco->registerSubsystem(tpc_endcap);

  return;
}

double TPC(PHG4Reco* g4Reco,
           double radius)
{
  bool OverlapCheck = Enable::OVERLAPCHECK || Enable::TPC_OVERLAPCHECK;
  bool AbsorberActive = Enable::ABSORBER || Enable::TPC_ABSORBER;

  PHG4TpcSubsystem* tpc = new PHG4TpcSubsystem("TPC");
  tpc->SetActive();
  tpc->SuperDetector("TPC");
  tpc->set_double_param("steplimits", 1);  // 1cm steps
  
  if (AbsorberActive)
    {
      tpc->SetAbsorberActive();
    }
  tpc->OverlapCheck(OverlapCheck);
  
  g4Reco->registerSubsystem(tpc);
  
  if (Enable::TPC_ENDCAP)
    {
      TPC_Endcaps(g4Reco);
    }
  
  radius = G4TPC::tpc_outer_radius;
  
  radius += no_overlapp;
  
  return radius;
}

void TPC_Cells()
{
  printf("RCC: tpc cells enabled\n");

  int verbosity = std::max(Enable::VERBOSITY, Enable::TPC_VERBOSITY);
  Fun4AllServer* se = Fun4AllServer::instance();

  //=========================
  // setup Tpc readout for filling cells
  // g4tpc/PHG4TpcElectronDrift uses
  // g4tpc/PHG4TpcPadPlaneReadout
  //=========================

  PHG4TpcPadPlane* padplane = new PHG4TpcPadPlaneReadout();
  padplane->Verbosity(verbosity);

  PHG4TpcElectronDrift* edrift = new PHG4TpcElectronDrift("PHG4TpcElectronDrift");
  printf("TPC_Cells: edrift built\n");
  printf("PHG4TpcElectronDrift reports its name as: %s\n", edrift->Name().c_str());

  edrift->Detector("TPC");
  printf("TPC_Cells: edrift successfully heard the word TPC\n");
  if (true) //set auto-advance and stepping of the directed laser, to fire in every frame
    {//eventually should be G4TPC::ENABLE_DIRECT_LASER
      edrift->setDirectLaser(true);
      edrift->directLaser->AimToThetaPhi(0.78,0.60);
(20,0.,6.28);
      //edrift->setDirectLaserAuto(true);
      //edrift->directLaser->SetPhiStepping(20,0.,6.28);
      //edrift->directLaser->SetThetaStepping(20,0.,1.57);
      //printf("all is well\n");
    }

  edrift->Verbosity(verbosity);
  printf("TPC_Cells: edrift finished\n");
  if( G4TPC::ENABLE_STATIC_DISTORTIONS || G4TPC::ENABLE_TIME_ORDERED_DISTORTIONS )
  {
    printf("distortions enabled\n");
    auto distortionMap = new PHG4TpcDistortion;
    distortionMap->set_do_static_distortions( G4TPC::ENABLE_STATIC_DISTORTIONS );
    distortionMap->set_static_distortion_filename( G4TPC::static_distortion_filename );

    distortionMap->set_do_time_ordered_distortions( G4TPC::ENABLE_TIME_ORDERED_DISTORTIONS );
    distortionMap->set_time_ordered_distortion_filename( G4TPC::time_ordered_distortion_filename );

    distortionMap->Init();
    edrift->setTpcDistortion( distortionMap );
  }

  // fudge factors to get drphi 150 microns (in mid and outer Tpc) and dz 500 microns cluster resolution
  // They represent effects not due to ideal gas properties and ideal readout plane behavior
  // defaults are 0.085 and 0.105, they can be changed here to get a different resolution
  edrift->registerPadPlane(padplane);
  se->registerSubsystem(edrift);
  printf("TPC_Cells: system registered\n");

  // The pad plane readout default is set in PHG4TpcPadPlaneReadout
  // We may want to change the number of inner layers, and can do that here
  padplane->set_int_param("tpc_minlayer_inner", G4MVTX::n_maps_layer + G4INTT::n_intt_layer);  // sPHENIX layer number of first Tpc readout layer
  padplane->set_int_param("ntpc_layers_inner", G4TPC::n_tpc_layer_inner);
  padplane->set_int_param("ntpc_phibins_inner", G4TPC::tpc_layer_rphi_count_inner);

  // Tpc digitizer
  //=========
  PHG4TpcDigitizer* digitpc = new PHG4TpcDigitizer();
  digitpc->SetTpcMinLayer(G4MVTX::n_maps_layer + G4INTT::n_intt_layer);
  double ENC = 670.0;  // standard
  digitpc->SetENC(ENC);
  double ADC_threshold = 4.0 * ENC;
  digitpc->SetADCThreshold(ADC_threshold);  // 4 * ENC seems OK
  digitpc->Verbosity(verbosity);
  cout << " Tpc digitizer: Setting ENC to " << ENC << " ADC threshold to " << ADC_threshold
       << " maps+Intt layers set to " << G4MVTX::n_maps_layer + G4INTT::n_intt_layer << endl;

  se->registerSubsystem(digitpc);

}

void TPC_Clustering()
{
  int verbosity = std::max(Enable::VERBOSITY, Enable::TPC_VERBOSITY);

  Fun4AllServer* se = Fun4AllServer::instance();

  //-------------
  // Cluster Hits
  //-------------

  // For the Tpc
  //==========
  auto tpcclusterizer = new TpcClusterizer;
  tpcclusterizer->Verbosity(verbosity);
  se->registerSubsystem(tpcclusterizer);

  auto tpcclustercleaner = new TpcClusterCleaner;
  tpcclustercleaner->Verbosity(verbosity);
  se->registerSubsystem(tpcclustercleaner);

  // space charge correction
  if( G4TPC::ENABLE_CORRECTIONS )
  {
    auto tpcSpaceChargeCorrection = new TpcSpaceChargeCorrection;
    tpcSpaceChargeCorrection->set_distortion_filename( G4TPC::correction_filename );
    tpcSpaceChargeCorrection->set_coordinates( G4TPC::correction_coordinates );
    se->registerSubsystem(tpcSpaceChargeCorrection);
  }

}


void TPC_QA()
{
  int verbosity = std::max(Enable::QA_VERBOSITY, Enable::TPC_VERBOSITY);

  Fun4AllServer* se = Fun4AllServer::instance();
  QAG4SimulationTpc * qa =  new QAG4SimulationTpc;
  qa->Verbosity(verbosity);
  se->registerSubsystem(qa);
}


#endif
