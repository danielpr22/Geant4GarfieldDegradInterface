#include "../include/GasBoxSD.hh"
#include "../include/DetectorConstruction.hh"
#include "../include/GasBoxHit.hh"

#include "G4Region.hh"
#include "G4UnitsTable.hh"
#include "G4String.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4HCofThisEvent.hh"
#include "G4TouchableHistory.hh"
#include "G4ParticleDefinition.hh"
#include "G4SDManager.hh"
#include "G4Gamma.hh"
#include "G4VProcess.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VVisManager.hh"
#include "G4Polyline.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"

GasBoxSD::GasBoxSD(G4String name) : G4VSensitiveDetector(name), fGasBoxHitsCollection(NULL){
    collectionName.insert("GBHC");
    GBHCID=-1;
}

GasBoxSD::~GasBoxSD(){}


void GasBoxSD::Initialize(G4HCofThisEvent * HCE){
    fGasBoxHitsCollection = new GasBoxHitsCollection(SensitiveDetectorName, collectionName[0]);
    if(GBHCID == -1){
        GBHCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
    }
    HCE->AddHitsCollection(GBHCID,fGasBoxHitsCollection);
    G4cout << "(Debug: GasBoxSD.cc) GasBoxSD Intialized!" << G4endl;
}

G4bool GasBoxSD::ProcessHits(G4Step* aStep, G4TouchableHistory* hist){
    G4Track* aTrack = aStep->GetTrack();

    if(aTrack->GetDefinition()->GetParticleName() == "e-"){
        G4cout << "(Debug: GasBoxSD.cc) GasBox Hit!" << G4endl;
        G4cout << "(Debug: GasBoxSD.cc)Particle ID: " << aTrack->GetTrackID() << G4endl;
        G4cout << "(Debug: GasBoxSD.cc) Energy electron: " << aTrack->GetKineticEnergy() << G4endl;
        G4cout << "(Debug: GasBoxSD.cc) Position electron: " << aTrack->GetVertexPosition().y() << G4endl;
    }
    return false;  
}

void GasBoxSD::EndOfEvent (G4HCofThisEvent * hce){
    auto HC = static_cast<GasBoxHitsCollection*>(hce->GetHC(GBHCID));
    int entries = HC->entries();
    G4cout << "(GasBoxSD.cc) Number of Electrons: " << entries << G4endl;
    if(entries > 0) {// If there is at least one electron created, a collision occurred
        G4cout << "(Debug: GasBox.cc) Gamma interaction occurred!" << G4endl;
        auto hit = (*HC)[0]; // We collect the first hit in the gas box 
        G4cout << "(Debug: GasBox.cc) Position of the interaction: " << (hit->GetPos().y())/mm << G4endl;
        if (abs((hit->GetPos().y())/mm) < (0.1)) { // If the ionization has happened close enough to the anode, we take it
            takeThisEvent = true; // This flag will be checked in the main file to go to the next configuration
        }
    }
    for(int i=0;i<entries;i++){
        auto hit = (*HC)[i];
        G4cout << "(Debug: GasBoxSD.cc) Hit position: " << G4BestUnit(hit->GetPos(), "Length") << ", Time: " << G4BestUnit(hit->GetTime(), "Time") << G4endl;
    }
}

