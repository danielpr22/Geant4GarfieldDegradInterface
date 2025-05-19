/**
 *\file DMPX_scan_angles_energies.cc
 *\brief Main program of DMPX_scan_angles_energies simulation
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

#include "include/DetectorConstruction.hh"
#include "include/PhysicsList.hh"
#include "include/MyUserActionInitialization.hh"
#include "include/GasModelParameters.hh"
#include "include/GasBoxSD.hh"


// Added for visualizing ROOT
// TApplication* rootApp = nullptr; 

int main(int argc, char** argv) {
  //rootApp = new TApplication("ROOT Application", &argc, argv);

  G4Random::setTheEngine(new CLHEP::RanecuEngine);
#ifdef G4MULTITHREADED
  G4MTRunManager* runManager = new G4MTRunManager();
  runManager->SetNumberOfThreads(2);
#else
  G4RunManager* runManager = new G4RunManager();
#endif
  G4cout << "(Debug: DMPX_scan_angles_energies.cc) G4RunManager is being created..." << G4endl;
    
  G4int randseed = atoi(argv[2]);
  G4Random::setTheSeed(randseed);
  G4cout << "(Debug: DMPX_scan_angles_energies.cc) Setting the Random seed: " << randseed << G4endl;
  
  G4cout << "(Debug: DMPX_scan_angles_energies.cc) Creation of the gas model parameter class" << G4endl;
  GasModelParameters* gmp = new GasModelParameters();
    
  G4cout << "(Debug: DMPX_scan_angles_energies.cc) Creation of DetectorConstruction" << G4endl;
  DetectorConstruction* detector = new DetectorConstruction(gmp);
  runManager->SetUserInitialization(detector);
  
  G4cout << "(Debug: DMPX_scan_angles_energies.cc) Creation of PhysicsList" << G4endl;
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
    UImanager->ApplyCommand("/control/execute scan_energies_angles.mac");

    GasBoxSD* gasBoxSD = detector->GetGasBoxSD();
    if (!gasBoxSD) {
        G4cerr << "(Error: DMPX_scan_angles_energies.cc) GasBoxSD not found!" << G4endl;
        return 1;
    }

    std::vector<double> energies = {3.0, 5.0, 10.0, 20.0, 25.0}; // in keV
    std::vector<double> angles = {65.0, 70.0, 75.0, 80.0, 85.0}; // in degrees

    for (double energy : energies) {
      for (double angle : angles) {
          UImanager->ApplyCommand("/gps/ene/mono " + std::to_string(energy) + " keV");
          UImanager->ApplyCommand("/gps/ang/minphi " + std::to_string(angle) + " deg");
          UImanager->ApplyCommand("/gps/ang/maxphi " + std::to_string(angle) + " deg");
  
          G4cout << "(Debug: DMPX_scan_angles_energies.cc) Running for energy: " << energy << " keV, angle: " << angle << " degrees" << G4endl;
  
          gasBoxSD->ResetGammaInteractionFlag(); // Reset the flag to false before starting
  
          bool interactionOccurred = false; 
          while (!interactionOccurred) {
              // Process one event
              runManager->BeamOn(1);

              G4cout << "(Debug: DMPX_scan_angles_energies.cc) Now shooting..." << G4endl;
  
              // Check if a gamma interaction occurred to move to the next configuration
              interactionOccurred = gasBoxSD->HasGammaInteractionOccurred();
              G4cout << "(Debug: DMPX_scan_angles_energies.cc) Interaction occurred: " << interactionOccurred << G4endl;
          }
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
      G4cout << "(Debug: DMPX_scan_angles_energies.cc) No random seed has been provided" << G4endl;
      delete runManager;
      return 0;
    }

    time_t start=time(0);
    UImanager->ApplyCommand(command + fileName);
    double duration = difftime(time(0),start);
    cout << "(Debug: DMPX_scan_angles_energies.cc) Simulation Time: " << duration << endl;
  }

  delete visManager; 
  delete runManager;
  return 0;
}
