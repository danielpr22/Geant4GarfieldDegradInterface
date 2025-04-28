#include "../include/MyUserActionInitialization.hh"
#include "../include/DetectorConstruction.hh"
#include "../include/RunAction.hh"
#include "../include/PrimaryGeneratorAction.hh"
#include "../include/EventAction.hh"
#include "../include/GasBoxSD.hh"
#include "../include/SteppingAction.hh"

#include "G4SDManager.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

MyUserActionInitialization::MyUserActionInitialization() {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

MyUserActionInitialization::~MyUserActionInitialization() {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void MyUserActionInitialization::Build() const {
	PrimaryGeneratorAction* primary = new PrimaryGeneratorAction();
	SetUserAction(primary);
	SteppingAction* stepAct = new SteppingAction();
	SetUserAction(stepAct);
	EventAction* evt = new EventAction();
	SetUserAction(evt);
	SetUserAction(new RunAction());
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void MyUserActionInitialization::BuildForMaster() const {
	SetUserAction(new RunAction());
}
