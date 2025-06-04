//Runaction
#include "../include/RunAction.hh"
#include "../include/PrimaryGeneratorAction.hh"
#include "../include/Analysis.hh"
#include "../include/EventAction.hh"
#include "../include/GasBoxSD.hh"

#include "G4Run.hh"
#include "Randomize.hh"
#include "G4AnalysisManager.hh"
#include "G4SDManager.hh"
#include "G4RunManager.hh"

RunAction::RunAction(){
  G4cout << "(Debug: RunAction.cc) Creating AnalysisManager..." << G4endl;
  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->SetVerboseLevel(1);
  analysisManager->SetActivation(true);  
  analysisManager->SetFileName("output.root"); 
  analysisManager->SetHistoDirectoryName("histo");
  analysisManager->SetNtupleDirectoryName("ntuple");
  analysisManager->SetNtupleActivation(false);
  G4cout << "(Debug: RunAction.cc) Creating RunAction..." << G4endl;
}

RunAction::~RunAction() { 
	G4cout << "(Debug: RunAction.cc) Deleting RunAction..." << G4endl;
}


void RunAction::BeginOfRunAction(const G4Run* aRun) {
  G4Random::showEngineStatus();

  G4cout << "(Debug: RunAction.cc) Starting run " << aRun->GetRunID() << G4endl;
  time_t currentTime;
  tm* ptm;
  time(&currentTime);
  ptm = localtime(&currentTime);
  G4cout << "(Debug: RunAction.cc) Time: " << asctime(ptm) << G4endl;

  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->OpenFile("output.root");  
}

void RunAction::EndOfRunAction(const G4Run* aRun) {
  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->Write();
  analysisManager->CloseFile();

  G4cout << "(Debug: RunAction.cc) End of run OK!" << G4endl;
  time_t currentTime;
  tm* ptm;
  time(&currentTime);
  ptm = localtime(&currentTime);
  G4cout << "(Debug: RunAction) Simulation finished!" << G4endl << "(Debug: RunAction.cc) Time: " << asctime(ptm)
         << G4endl;
  G4Random::showEngineStatus();
}
