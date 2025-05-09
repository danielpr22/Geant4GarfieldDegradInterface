#include "../include/DetectorMessenger.hh"
#include "../include/DetectorConstruction.hh"

#include "G4UnitsTable.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWith3Vector.hh"
#include "G4SystemOfUnits.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorMessenger::DetectorMessenger(DetectorConstruction* HPGeDet)
    : detector(HPGeDet) {
    miniDir = new G4UIdirectory("/DMPX/");
    miniDir->SetGuidance("DMPX specific controls");

    geometryDir = new G4UIdirectory("/DMPX/geometry/");
    geometryDir->SetGuidance("DMPX geometry specific controls");

    setGasPressCmd =
      new G4UIcmdWithADoubleAndUnit("/DMPX/geometry/SetGasPressure", this);

    setGasPressCmd->SetGuidance("Set gas pressure.");
    setGasPressCmd->SetUnitCategory("Pressure");
    setGasPressCmd->SetDefaultValue(0.3 * bar);
    setGasPressCmd->AvailableForStates(G4State_PreInit, G4State_Idle); 
    
    pressure = 0.0 * bar; 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorMessenger::~DetectorMessenger() {
    delete miniDir;
    delete geometryDir;
    delete setGasPressCmd;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorMessenger::SetNewValue(G4UIcommand* command, G4String newValues) {
  if (command == setGasPressCmd)
    pressure = setGasPressCmd->GetNewDoubleValue(newValues);
    detector->SetGasPressure(pressure);
    G4cout << "(Debug: DetectorMessenger.cc) Setting gas pressure to " << 
    G4BestUnit(pressure, "Pressure") << G4endl;
}
