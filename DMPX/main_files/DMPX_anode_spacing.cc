/**
 *\file DMPX_anode_spacing.cc
 *\brief Main program of DMPX_anode_spacing simulation
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
  G4cout << "(Debug: DMPX_anode_spacing.cc) G4RunManager is being created..." << G4endl;
    
  G4int randseed = atoi(argv[2]);
  G4Random::setTheSeed(randseed);
  G4cout << "(Debug: DMPX_anode_spacing.cc) Setting the Random seed: " << randseed << G4endl;
  
  G4cout << "(Debug: DMPX_anode_spacing.cc) Creation of the gas model parameter class" << G4endl;
  GasModelParameters* gmp = new GasModelParameters();
    
  G4cout << "(Debug: DMPX_anode_spacing.cc) Creation of DetectorConstruction" << G4endl;
  DetectorConstruction* detector = new DetectorConstruction(gmp);
  runManager->SetUserInitialization(detector);
  
  G4cout << "(Debug: DMPX_anode_spacing.cc) Creation of PhysicsList" << G4endl;
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
    UImanager->ApplyCommand("/control/execute run_files/anode_spacing.mac");

    GasBoxSD* gasBoxSD = detector->GetGasBoxSD();
    if (!gasBoxSD) {
        G4cerr << "(Error: DMPX_anode_spacing.cc) GasBoxSD not found!" << G4endl;
        return 1;
    }


    std::vector<double> spacing = {1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0}; // in mm

    for (double spc : spacing) {
        UImanager->ApplyCommand("/DMPX/geometry/SetAnodesSpacing " + std::to_string(spc) + " mm");

        G4cout << "(Debug: DMPX_anode_spacing.cc) Running for spacing: " << spc << " mm" << G4endl;

        gasBoxSD->ResetGammaInteractionFlag(); // Reset the flag to false before starting

        bool interactionOccurred = false; 
        while (!interactionOccurred) {
            // Process one event
            runManager->BeamOn(1);

            G4cout << "(Debug: DMPX_anode_spacing.cc) Now shooting..." << G4endl;

            // Check if a gamma interaction occurred to move to the next configuration
            interactionOccurred = gasBoxSD->HasGammaInteractionOccurred();
            G4cout << "(Debug: DMPX_anode_spacing.cc) Interaction occurred: " << interactionOccurred << G4endl;
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
      G4cout << "(Debug: DMPX_anode_spacing.cc) No random seed has been provided" << G4endl;
      delete runManager;
      return 0;
    }

    time_t start=time(0);
    UImanager->ApplyCommand(command + fileName);
    double duration = difftime(time(0),start);
    cout << "(Debug: DMPX_anode_spacing.cc) Simulation Time: " << duration << endl;
  }

  delete visManager; 
  delete runManager;
  return 0;
}
