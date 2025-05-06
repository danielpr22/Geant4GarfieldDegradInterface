//
//  PrimaryGeneratorAction.cpp
//  Xenon
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

PrimaryGeneratorAction::PrimaryGeneratorAction(){
    particleGun = new G4GeneralParticleSource();
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
    delete particleGun;
    G4cout << "(Debug: PrimaryGeneratorAction.cc) Deleting PrimaryGeneratorAction" << G4endl;
} 

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
    particleGun->GeneratePrimaryVertex(anEvent);
}

