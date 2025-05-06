/**
 *\file Xenon.cc
 *\brief Main program of Xenon simulation
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
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4UIterminal.hh"
#include "G4UItcsh.hh"
#include "G4VSteppingVerbose.hh"
#include "Randomize.hh" 
#include "TApplication.h" 

#include "include/DetectorConstruction.hh"
#include "include/PhysicsList.hh"
#include "include/MyUserActionInitialization.hh"
#include "include/GasModelParameters.hh"

// Added for visualizing ROOT
TApplication* rootApp = nullptr; 

int main(int argc, char** argv) {
  rootApp = new TApplication("ROOT Application", &argc, argv);

  G4Random::setTheEngine(new CLHEP::RanecuEngine);
#ifdef G4MULTITHREADED
  G4MTRunManager* runManager = new G4MTRunManager();
  runManager->SetNumberOfThreads(2);
#else
  G4RunManager* runManager = new G4RunManager();
#endif
  G4cout << "(Debug: Xenon.cc) G4RunManager is being created..." << G4endl;
    
  G4int randseed = atoi(argv[2]);
  G4Random::setTheSeed(randseed);
  G4cout << "(Debug: Xenon.cc) Setting the Random seed: " << randseed << G4endl;
  
  G4cout << "(Debug: Xenon.cc) Creation of the gas model parameter class" << G4endl;
  GasModelParameters* gmp = new GasModelParameters();
    
  G4cout << "(Debug: Xenon.cc) Creation of DetectorConstruction" << G4endl;
  DetectorConstruction* detector = new DetectorConstruction(gmp);
  runManager->SetUserInitialization(detector);
  
  G4cout << "(Debug: Xenon.cc) Creation of PhysicsList" << G4endl;
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
    //#ifdef G4UI_USE
    G4UIExecutive* ui = new G4UIExecutive(argc, argv);
    //#ifdef G4VIS_USE
    UImanager->ApplyCommand("/control/execute vis.mac");
    //#endif

    ui->SessionStart();
    delete ui;
    //#endif
  } else  //! batch mode:
  {
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    if (argc < 3) {
      G4cout << "(Debug: Xenon.cc) No random seed has been provided" << G4endl;
      delete runManager;
      return 0;
    }

    time_t start=time(0);
    UImanager->ApplyCommand(command + fileName);
    double duration = difftime(time(0),start);
    cout << "(Debug: Xenon.cc) Simulation Time: " << duration << endl;
  }

  rootApp->Run();

  delete visManager; 
  delete runManager;
  delete rootApp; 
  return 0;
}
