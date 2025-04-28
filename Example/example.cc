#include "DetectorConstruction.hh"
#include "PhysicsList.hh"
#include "MyUserActionInitialization.hh"

#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"

int main()
{
    auto runManager = G4RunManagerFactory::CreateRunManager(); 

    runManager->SetUserInitialization(new DetectorConstruction);
    runManager->SetUserInitialization(new PhysicsList);
    runManager->SetUserInitialization(new MyUserActionInitialization);

    runManager->Initialize(); 

    G4UImanager* UI = G4UImanager::GetUIpointer();
    UI->ApplyCommand("/run/verbose 1");
    UI->ApplyCommand("/event/verbose 1");
    UI->ApplyCommand("/tracking/verbose 1");

    int nbEvents = 3
    runManager->BeamOn(nbEvents);

    delete runManager;
    return 0;
}