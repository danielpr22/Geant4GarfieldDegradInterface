/**
 *\file DMPX.cc
 *\brief Main program of DMPX simulation
 *\author Daniel Perales Rios (daniel.peralesrios@epfl.ch)
 *\date April 2025


You can find the Wiki on the subversion repository:
https://svs.icts.kuleuven.be/projects/svs_project014/wiki/Wiki
 */

#include <ctime>
#include <cstdio>
#include <time.h>

#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4UIterminal.hh"
#include "G4UItcsh.hh"
#include "G4VSteppingVerbose.hh"
#include "Randomize.hh" 

#include "DetectorConstruction.hh"
#include "PhysicsList.hh"
#include "MyUserActionInitialization.hh"
#include "GasModelParameters.hh"

int main(int argc, char** argv) {
  G4Random::setTheEngine(new CLHEP::RanecuEngine);

  G4RunManager* runManager = new G4RunManager();
  G4cout << "(Debug: DMPX.cc) Creation of G4RunManager..." << G4endl;
  
  G4int randseed = atoi(argv[2]);
  G4Random::setTheSeed(randseed);
  G4cout << "(Debug: DMPX.cc) Setting the Random seed: " << randseed << G4endl;
  
  G4cout << "(Debug: DMPX.cc) Creation of the gas model parameter class..." << G4endl;
  GasModelParameters* gmp = new GasModelParameters();
    
  G4cout << "(Debug: DMPX.cc) Creation of DetectorConstruction..." << G4endl;
  DetectorConstruction* detector = new DetectorConstruction(gmp);
  runManager->SetUserInitialization(detector);

  G4cout << "(Debug: DMPX.cc) Creation of PhysicsList..." << G4endl;
  PhysicsList* physics = new PhysicsList();
  runManager->SetUserInitialization(physics);
  
  runManager->SetUserInitialization(new MyUserActionInitialization());
 
   // get the pointer to the User Interface manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer();

  G4VisManager* visManager = new G4VisExecutive();
  visManager->Initialize();

  //runManager->Initialize();

  if (argc == 1)  //! define UI terminal for interactive mode:
  {
    G4UIExecutive* ui = new G4UIExecutive(argc, argv);
    UImanager->ApplyCommand("/control/execute vis.mac");
    ui->SessionStart();
    delete ui;
  } else  //! batch mode:
  {
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    if (argc < 3) {
      G4cout << "(Debug: DMPX.cc) No random seed has been provided!" << G4endl;
      delete runManager;
      return 0;
    }

    time_t start=time(0);
    UImanager->ApplyCommand(command + fileName);
    double duration = difftime(time(0),start);
    G4cout << "(Debug: DMPX.cc) Simulation Time: " << duration << G4endl;
  }

  delete visManager;
  delete runManager;
  return 0;
}
