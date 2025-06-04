#include "../include/EventAction.hh"
#include "../include/DegradModel.hh"
#include "../include/DetectorConstruction.hh"
#include "../include/Analysis.hh"
#include "../include/HeedModel.hh"
#include "../include/RunAction.hh"

#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4ios.hh"
#include "G4SDManager.hh"
#include "G4Threading.hh"
#include "G4VPhysicalVolume.hh"
#include "G4GlobalFastSimulationManager.hh"

EventAction::EventAction() : eventStarted(false) {
    energyPrimary = 0.0;
    distanceAnodesSource = 0.0;
    shotNumber = 0;
    numberOfEvents = 0; 
}

EventAction::~EventAction() {
	G4cout << "(Debug: EventAction.cc) Deleting EventAction..." << G4endl;
}

void EventAction::BeginOfEventAction(const G4Event *ev) {
    eventStarted = true; 
    shotNumber++; 
    G4cout << "(Debug: EventAction.cc) Beginning of event " << shotNumber << G4endl;


    DegradModel* dm = (DegradModel*)(G4GlobalFastSimulationManager::GetInstance()->GetFastSimulationModel("DegradModel"));
    if(dm)
        dm->Reset();

    const G4PrimaryVertex* primaryVertex = ev->GetPrimaryVertex();
    if (primaryVertex) {
        G4ThreeVector position = primaryVertex->GetPosition(); // Get the position of the primary vertex
        // This distance will be used for the calculation of the electric field inside Degrad
        distanceAnodesSource = position.getY();  
        const G4PrimaryParticle* primaryParticle = primaryVertex->GetPrimary();
        G4cout << "(Debug: EventAction.cc) The primary vertex is: " << position.getY() << G4endl;

        if (primaryParticle) {
            energyPrimary = primaryParticle->GetKineticEnergy();
            G4cout << "(Debug: EventAction.cc) Primary particle energy: " << energyPrimary / keV << " keV" << G4endl;
        }
    }
}


void EventAction::EndOfEventAction(const G4Event *evt) {
    eventStarted = false; 
}

