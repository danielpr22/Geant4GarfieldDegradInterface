#include "../include/MyUserActionInitialization.hh"
#include "../include/DetectorConstruction.hh"
#include "../include/RunAction.hh"
#include "../include/PrimaryGeneratorAction.hh"
#include "../include/EventAction.hh"
#include "../include/GasBoxSD.hh"

#include "G4SDManager.hh"

MyUserActionInitialization::MyUserActionInitialization(){}

MyUserActionInitialization::~MyUserActionInitialization(){}

void MyUserActionInitialization::Build() const {
	PrimaryGeneratorAction* primary = new PrimaryGeneratorAction(); // For the particle gun
	SetUserAction(primary);
	EventAction* evt = new EventAction();
	SetUserAction(evt);
	SetUserAction(new RunAction());
}

void MyUserActionInitialization::BuildForMaster() const {
	SetUserAction(new RunAction());
}
