#include "../include/GasBoxSD.hh"
#include "../include/DetectorConstruction.hh"

#include "G4Region.hh"
#include "G4String.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4HCofThisEvent.hh"
#include "G4TouchableHistory.hh"
#include "G4SDManager.hh"
#include "G4VProcess.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VVisManager.hh"
#include "G4Polyline.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"

GasBoxSD::GasBoxSD(G4String name) : G4VSensitiveDetector(name),
    fGasBoxHitsCollection(NULL), fGarfieldExcitationHitsCollection(NULL) {
    collectionName.insert("GBHCID");
    
    GBHCID=-1;
}

GasBoxSD::~GasBoxSD() {}


void GasBoxSD::Initialize(G4HCofThisEvent * HCE) {
    fGasBoxHitsCollection = new GasBoxHitsCollection(SensitiveDetectorName, collectionName[0]);
    fGarfieldExcitationHitsCollection = new GarfieldExcitationHitsCollection(SensitiveDetectorName, collectionName[1]);
    if(GBHCID==-1){
        GBHCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[1]);
    }
    HCE->AddHitsCollection(GBHCID,fGasBoxHitsCollection);

    G4cout << "(Debug: GasBoxSD.cc) GasBoxSD Intialized!" << G4endl;
}

G4bool GasBoxSD::ProcessHits(G4Step* aStep, G4TouchableHistory* hist){
    G4Track* aTrack = aStep->GetTrack();

    if(aTrack->GetDefinition()->GetParticleName() == "e-"){
        G4cout << "(Debug: GasBoxSD.cc) GasBox Hit!" << G4endl;
        G4cout << "(Debug: GasBoxSD.cc) Particle ID: " << aTrack->GetTrackID() << G4endl;
        G4cout << "(Debug: GasBoxSD.cc) Energy electron: " << aTrack->GetKineticEnergy() << G4endl;
        return true;
    }

    return false;  
}


void GasBoxSD::EndOfEvent (G4HCofThisEvent * hce){// Hits collection of the event
    auto HC = static_cast<GasBoxHitsCollection*>(hce->GetHC(GBHCID));
    int entries = HC->entries();
    G4cout << "(Debug: GasBoxSD.cc) Number of Electrons (entries): " << entries << G4endl;
    for(int i=0;i<entries;i++){
        auto hit = (*HC)[i];
        G4cout << "(Debug: GasBoxSD.cc) Hit position (entries): " << hit->GetPos() << ", Time: " << hit->GetTime() << G4endl;
    }
    DrawAll();
}

void GasBoxSD::DrawAll() {}
