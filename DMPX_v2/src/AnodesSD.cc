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

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

AnodeSD::AnodeSD(G4String name) : G4VSensitiveDetector(name), fAnodeHitsCollection(NULL) {
    collectionName.insert("SHC");
    AWHCID=-1;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

AnodeSD::~AnodeSD(){}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void AnodeSD::Initialize(G4HCofThisEvent * HCE) {
    fAnodeHitsCollection = new GasBoxHitsCollection(SensitiveDetectorName, collectionName[0]);
    if(AWHCID==-1){
        AWHCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
    }
    HCE->AddHitsCollection(AWHCID,fAnodeHitsCollection);

    G4cout << "(Debug: AnodesSD.cc) AnodeSD Intialized!" << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4bool AnodeSD::ProcessHits(G4Step* aStep, G4TouchableHistory* hist) {
    G4Track* aTrack = aStep->GetTrack();
    G4StepPoint* thePostPoint = aStep->GetPostStepPoint();

    if(aStep->IsFirstStepInVolume()){
        G4cout << "(Debug: AnodeSD.cc) Anode Wall Hit!" << G4endl;
        G4cout << "(Debug: AnodeSD.cc) Particle ID: " << aTrack->GetTrackID() << G4endl;
        G4cout << "(Debug: AnodeSD.cc) Energy loss through gas: " << aTrack->GetVertexKineticEnergy() - aTrack->GetKineticEnergy() << G4endl;
        return true;
    }
    return false;      
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void AnodeSD::EndOfEvent (G4HCofThisEvent * hce) {}