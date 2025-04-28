#include "../include/EventAction.hh"
#include "../include/DetectorConstruction.hh"
#include "../include/Analysis.hh"
#include "../include/SteppingAction.hh"
#include "../include/RunAction.hh"

#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4ios.hh"
#include "G4AnalysisManager.hh"
#include "G4SDManager.hh"
#include "G4Threading.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

auto analysisManager = G4AnalysisManager::Instance();

EventAction::EventAction() {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::~EventAction() {
	G4cout << "(Debug: EventAction.cc) Deleting EventAction" << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::BeginOfEventAction(const G4Event *ev) {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::EndOfEventAction(const G4Event *evt) {}

