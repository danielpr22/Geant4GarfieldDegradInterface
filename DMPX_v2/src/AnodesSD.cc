#include "../include/AnodesSD.hh"
#include "../include/DetectorConstruction.hh"
#include "../include/GasBoxHit.hh"

#include "G4Region.hh"
#include "G4String.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4HCofThisEvent.hh"
#include "G4TouchableHistory.hh"
#include "G4SDManager.hh"
#include "G4VProcess.hh"
#include "G4VPhysicalVolume.hh"

AnodesSD::AnodesSD(G4String name) : G4VSensitiveDetector(name), fAnodesHitsCollection(NULL){
    collectionName.insert("AHC");
    AHCID=-1;
}

AnodesSD::~AnodesSD(){}


void AnodesSD::Initialize(G4HCofThisEvent * HCE){
    fAnodesHitsCollection = new AnodesHitsCollection(SensitiveDetectorName, collectionName[0]);
    if(AHCID==-1){
        AHCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
    }
    HCE->AddHitsCollection(AHCID,fAnodesHitsCollection);

    G4cout << "(Debug: AnodesSD.cc) AnodesSD Intialized!" << G4endl;
}

G4bool AnodesSD::ProcessHits(G4Step* aStep, G4TouchableHistory* hist){
    G4Track* aTrack = aStep->GetTrack();
    G4StepPoint* thePostPoint = aStep->GetPostStepPoint();

    if(aStep->IsFirstStepInVolume()){
        G4cout << "(Debug: AnodesSD.cc) Anodes Wall Hit!" << G4endl;
        G4cout << "(Debug: AnodesSD.cc) Particle ID: " << aTrack->GetTrackID() << G4endl;
        G4cout << "(Debug: AnodesSD.cc) Energy loss through gas: " << aTrack->GetVertexKineticEnergy() - aTrack->GetKineticEnergy() << G4endl;
        return true;
    }
    return false;      
}

void AnodesSD::EndOfEvent (G4HCofThisEvent * hce){
   
}