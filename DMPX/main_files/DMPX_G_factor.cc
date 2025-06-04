/**
 *\file DMPX_G_factor.cc
 *\brief Main program of DMPX_G_factor simulation
 *\author Claire Couratin
 *\date December 2014


You can find the Wiki on the subversion repository:
https://svs.icts.kuleuven.be/projects/svs_project014/wiki/Wiki
 */
#include <ctime>
#include <cstdio>
#include <time.h>

#include "G4RunManager.hh"
#include "G4MTRunManager.hh"
#include "G4UImanager.hh"
#include "G4SDManager.hh"
#include "G4VisExecutive.hh"
#include "G4Step.hh"
#include "G4UIExecutive.hh"
#include "G4UIterminal.hh"
#include "G4UItcsh.hh"
#include "G4ios.hh"
#include "G4VSteppingVerbose.hh"
#include "Randomize.hh" 
#include "TApplication.h" 

#include "../include/DetectorConstruction.hh"
#include "../include/PhysicsList.hh"
#include "../include/MyUserActionInitialization.hh"
#include "../include/GasModelParameters.hh"
#include "../include/GasBoxSD.hh"



int main(int argc, char** argv) {

  G4Random::setTheEngine(new CLHEP::RanecuEngine);
#ifdef G4MULTITHREADED
  G4MTRunManager* runManager = new G4MTRunManager();
  runManager->SetNumberOfThreads(2);
#else
  G4RunManager* runManager = new G4RunManager();
#endif
  G4cout << "(Debug: DMPX_G_factor.cc) G4RunManager is being created..." << G4endl;
    
  G4int randseed = atoi(argv[2]);
  G4Random::setTheSeed(randseed);
  G4cout << "(Debug: DMPX_G_factor.cc) Setting the Random seed: " << randseed << G4endl;
  
  G4cout << "(Debug: DMPX_G_factor.cc) Creation of the gas model parameter class" << G4endl;
  GasModelParameters* gmp = new GasModelParameters();
    
  G4cout << "(Debug: DMPX_G_factor.cc) Creation of DetectorConstruction" << G4endl;
  DetectorConstruction* detector = new DetectorConstruction(gmp);
  runManager->SetUserInitialization(detector);
  
  G4cout << "(Debug: DMPX_G_factor.cc) Creation of PhysicsList" << G4endl;
  PhysicsList* physics = new PhysicsList();
  runManager->SetUserInitialization(physics);
  
  runManager->SetUserInitialization(new MyUserActionInitialization());
 
  // User interface manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer();

  // Visual manager
  G4VisManager* visManager = new G4VisExecutive();
  visManager->Initialize();

  if (argc == 1)  //! define UI terminal for interactive mode:
  {
    G4UIExecutive* ui = new G4UIExecutive(argc, argv);
    UImanager->ApplyCommand("/control/execute run_files/G_factor.mac");

    GasBoxSD* gasBoxSD = detector->GetGasBoxSD();
    if (!gasBoxSD) {
        G4cerr << "(Error: DMPX_G_factor.cc) GasBoxSD not found!" << G4endl;
        return 1;
    }

    // std::vector<double> voltages = {-100.0, -200.0, -300.0, -400.0, -500.0, -600.0, -700.0, -800.0, -900.0, -1000.0, 
    //   -1100.0, -1200.0, -1300.0, -1400.0, -1500.0, -1600.0, -1700.0, -1800.0, -1900.0, -2000.0, -2100.0, -2200.0, -2300.0, -2400.0, -2500.0, 
    //   -2600.0, -2700.0, -2800.0, -2900.0, -3000.0, -3100.0, -3200.0, -3300.0, -3400.0, -3500.0}; // in V


    std::vector<double> voltages = {-3100.0, -3200.0, -3300.0, -3400.0, -3500.0}; // in V

    for (double voltage : voltages) {
        UImanager->ApplyCommand("/gasModelParameters/heed/voltagecathodeplane " + std::to_string(voltage) + " V");

        G4cout << "(Debug: DMPX_G_factor.cc) Running for voltage: " << voltage << " V" << G4endl;

        gasBoxSD->ResetGammaInteractionFlag(); // Reset the flag to false before starting

        bool interactionOccurred = false; 
        while (!interactionOccurred) {
            // Process one event
            runManager->BeamOn(1);

            G4cout << "(Debug: DMPX_G_factor.cc) Now shooting..." << G4endl;

            // Check if a gamma interaction occurred to move to the next configuration
            interactionOccurred = gasBoxSD->HasGammaInteractionOccurred();
            G4cout << "(Debug: DMPX_G_factor.cc) Interaction occurred: " << interactionOccurred << G4endl;
      }
  }

  // Initialize the UI manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer(); 

  // Start the Geant4 UI
  ui->SessionStart();
  delete ui;

  } else  //! batch mode:
  {
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    if (argc < 3) {
      G4cout << "(Debug: DMPX_G_factor.cc) No random seed has been provided" << G4endl;
      delete runManager;
      return 0;
    }

    time_t start=time(0);
    UImanager->ApplyCommand(command + fileName);
    double duration = difftime(time(0),start);
    cout << "(Debug: DMPX_G_factor.cc) Simulation Time: " << duration << endl;
  }

  delete visManager; 
  delete runManager;
  return 0;
}
