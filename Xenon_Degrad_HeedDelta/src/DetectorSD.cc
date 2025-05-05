#include "DetectorSD.hh"
#include "DetectorConstruction.hh"
#include "GasBoxHit.hh"

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

DetectorSD::DetectorSD(G4String name) : G4VSensitiveDetector(name), fGasBoxHitsCollection(NULL) {
    collectionName.insert("SHC");
    SHCID=-1;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorSD::~DetectorSD(){}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorSD::Initialize(G4HCofThisEvent * HCE) {
    fGasBoxHitsCollection = new GasBoxHitsCollection(SensitiveDetectorName, collectionName[0]);
    if(SHCID==-1){
        SHCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
    }
    HCE->AddHitsCollection(SHCID,fGasBoxHitsCollection);

    G4cout << "(Debug: DetectorSD.cc) DetectorSD Intialized!" << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4bool DetectorSD::ProcessHits(G4Step* aStep, G4TouchableHistory* hist) {
    G4Track* aTrack = aStep->GetTrack();
    G4StepPoint* thePostPoint = aStep->GetPostStepPoint();

    if(aStep->IsFirstStepInVolume()){
        G4cout << "(Debug: DetectorSD.cc) Detector Wall Hit!" << G4endl;
        G4cout << "(Debug: DetectorSD.cc) Particle ID: " << aTrack->GetTrackID() << G4endl;
        G4cout << "(Debug: DetectorSD.cc) Energy loss through gas: " << aTrack->GetVertexKineticEnergy() - aTrack->GetKineticEnergy() << G4endl;
        return true;
    }
    return false;      
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorSD::EndOfEvent (G4HCofThisEvent * hce) {}