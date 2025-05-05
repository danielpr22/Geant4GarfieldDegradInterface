#include "MyUserActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "RunAction.hh"
#include "PrimaryGeneratorAction.hh"
#include "EventAction.hh"
#include "GasBoxSD.hh"
#include "SteppingAction.hh"

#include "G4SDManager.hh"

MyUserActionInitialization::MyUserActionInitialization(){}

MyUserActionInitialization::~MyUserActionInitialization(){}

void MyUserActionInitialization::Build() const {
	PrimaryGeneratorAction* primary = new PrimaryGeneratorAction();
	SetUserAction(primary);
	EventAction* evt = new EventAction();
	SetUserAction(evt);
	SetUserAction(new RunAction());
}

void MyUserActionInitialization::BuildForMaster() const {
	SetUserAction(new RunAction());
}
