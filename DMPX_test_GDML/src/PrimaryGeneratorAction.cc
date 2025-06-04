//
//  PrimaryGeneratorAction.cpp
//  ALICE
//
//  Created by Lennert De Keukeleere on 25/10/2018.
//

#include "../include/PrimaryGeneratorAction.hh"
#include "../include/DetectorConstruction.hh"
#include "../include/RunAction.hh"

#include "G4Event.hh"
#include "G4RandomDirection.hh"
#include "G4RunManager.hh"
#include "G4ThreeVector.hh"
#include "G4Geantino.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::PrimaryGeneratorAction(){
    particleGun = new G4GeneralParticleSource();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
    delete particleGun;
    G4cout << "(Debug: PrimaryGeneratorAction.cc) Deleting PrimaryGeneratorAction..." << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
    particleGun->GeneratePrimaryVertex(anEvent);
}

