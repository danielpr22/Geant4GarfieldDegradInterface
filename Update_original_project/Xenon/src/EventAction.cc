#include "EventAction.hh"
#include "DegradModel.hh"
#include "DetectorConstruction.hh"
#include "Analysis.hh"
#include "SteppingAction.hh"
#include "RunAction.hh"

#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4ios.hh"
#include "G4SDManager.hh"
#include "G4Threading.hh"
#include "G4VPhysicalVolume.hh"
#include "G4GlobalFastSimulationManager.hh"

EventAction::EventAction() {}

EventAction::~EventAction() {
	G4cout << "(Debug: EventAction.cc) Deleting EventAction..." << G4endl;
}

void EventAction::BeginOfEventAction(const G4Event *ev) {
    DegradModel* dm = (DegradModel*)(G4GlobalFastSimulationManager::GetInstance()->GetFastSimulationModel("DegradModel"));
    if(dm)
        dm->Reset();
}

void EventAction::EndOfEventAction(const G4Event *evt) {}
