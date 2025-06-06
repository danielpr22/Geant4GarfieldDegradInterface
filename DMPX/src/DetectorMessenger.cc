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

    // Setting the world half length for the simulation
    setWorldHalfLengthCmd =
      new G4UIcmdWithADoubleAndUnit("/DMPX/geometry/SetWorldHalfLength", this);
    setWorldHalfLengthCmd->SetGuidance("Set world half length.");
    setWorldHalfLengthCmd->SetUnitCategory("Length");
    setWorldHalfLengthCmd->SetDefaultValue(0.2 * m);
    setWorldHalfLengthCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    setCheckOverlapsCmd =
      new G4UIcmdWithABool("/DMPX/geometry/CheckOverlaps", this);
    setCheckOverlapsCmd->SetGuidance("Check overlaps in the geometry.");
    setCheckOverlapsCmd->SetDefaultValue(false);
    setCheckOverlapsCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    // Retrieving the gas pressure
    setGasPressCmd =
      new G4UIcmdWithADoubleAndUnit("/DMPX/geometry/SetGasPressure", this);
    setGasPressCmd->SetGuidance("Set gas pressure.");
    setGasPressCmd->SetUnitCategory("Pressure");
    setGasPressCmd->SetDefaultValue(1.0 * bar);
    setGasPressCmd->AvailableForStates(G4State_PreInit, G4State_Idle); 

    // Setting the percentages for the gas mixture (ad hoc commands)
    setKryptonPercentageCmd =
      new G4UIcmdWithADouble("/DMPX/geometry/SetKryptonPercentage", this);
    setKryptonPercentageCmd->SetGuidance("Set the percentage of Krypton in the gas mixture.");
    setKryptonPercentageCmd->SetDefaultValue(90.0);
    setKryptonPercentageCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    setCH4PercentageCmd =
      new G4UIcmdWithADouble("/DMPX/geometry/SetCH4Percentage", this);
    setCH4PercentageCmd->SetGuidance("Set the percentage of CH4 in the gas mixture.");
    setCH4PercentageCmd->SetDefaultValue(10.0);
    setCH4PercentageCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    // Setting the dimensions of the gas box
    setGasBoxLengthXCmd = 
      new G4UIcmdWithADoubleAndUnit("/DMPX/geometry/SetGasBoxLengthX", this);
    setGasBoxLengthXCmd->SetGuidance("Set gas box length in X direction.");
    setGasBoxLengthXCmd->SetUnitCategory("Length");
    setGasBoxLengthXCmd->SetDefaultValue(160.0 * mm);
    setGasBoxLengthXCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    setGasBoxLengthYCmd = 
      new G4UIcmdWithADoubleAndUnit("/DMPX/geometry/SetGasBoxLengthY", this);
    setGasBoxLengthXCmd->SetGuidance("Set gas box length in Y direction.");
    setGasBoxLengthXCmd->SetUnitCategory("Length");
    setGasBoxLengthXCmd->SetDefaultValue(5.0 * mm);
    setGasBoxLengthXCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    setGasBoxLengthZCmd = 
      new G4UIcmdWithADoubleAndUnit("/DMPX/geometry/SetGasBoxLengthZ", this);
    setGasBoxLengthXCmd->SetGuidance("Set gas box length in Z direction.");
    setGasBoxLengthXCmd->SetUnitCategory("Length");
    setGasBoxLengthXCmd->SetDefaultValue(32.0 * mm);
    setGasBoxLengthXCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    // Setting the position of the center of the gas box (default: 0, 0, 0)
    setGasBoxCenterPositionXCmd = 
      new G4UIcmdWithADoubleAndUnit("/DMPX/geometry/SetGasBoxCenterPositionX", this);
    setGasBoxCenterPositionXCmd->SetGuidance("Set gas box center position in X direction.");
    setGasBoxCenterPositionXCmd->SetUnitCategory("Length");
    setGasBoxCenterPositionXCmd->SetDefaultValue(0.0 * mm);
    setGasBoxCenterPositionXCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    setGasBoxCenterPositionYCmd = 
      new G4UIcmdWithADoubleAndUnit("/DMPX/geometry/SetGasBoxCenterPositionY", this);
    setGasBoxCenterPositionYCmd->SetGuidance("Set gas box center position in Y direction.");
    setGasBoxCenterPositionYCmd->SetUnitCategory("Length");
    setGasBoxCenterPositionYCmd->SetDefaultValue(0.0 * mm);
    setGasBoxCenterPositionYCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    setGasBoxCenterPositionZCmd = 
      new G4UIcmdWithADoubleAndUnit("/DMPX/geometry/SetGasBoxCenterPositionZ", this);
    setGasBoxCenterPositionZCmd->SetGuidance("Set gas box center position in Z direction.");
    setGasBoxCenterPositionZCmd->SetUnitCategory("Length");
    setGasBoxCenterPositionZCmd->SetDefaultValue(0.0 * mm);
    setGasBoxCenterPositionZCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    // Settings for the anodes and cathodes 
    setAnodesHalfLengthCmd = 
      new G4UIcmdWithADoubleAndUnit("/DMPX/geometry/SetAnodesHalfLength", this);
    setAnodesHalfLengthCmd->SetGuidance("Set anodes half length (in the Z direction).");
    setAnodesHalfLengthCmd->SetUnitCategory("Length");
    setAnodesHalfLengthCmd->SetDefaultValue(16.0 * mm);
    setAnodesHalfLengthCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    setAnodesRCmd = 
      new G4UIcmdWithADoubleAndUnit("/DMPX/geometry/SetAnodesR", this);
    setAnodesRCmd->SetGuidance("Set anodes radius.");
    setAnodesRCmd->SetUnitCategory("Length");
    setAnodesRCmd->SetDefaultValue(0.1 * mm);
    setAnodesRCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    setAnodesSpacingCmd = 
      new G4UIcmdWithADoubleAndUnit("/DMPX/geometry/SetAnodesSpacing", this);
    setAnodesSpacingCmd->SetGuidance("Set anodes spacing.");
    setAnodesSpacingCmd->SetUnitCategory("Length");
    setAnodesSpacingCmd->SetDefaultValue(2.0 * mm);
    setAnodesSpacingCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    setNbOfAnodesCmd = 
      new G4UIcmdWithAnInteger("/DMPX/geometry/SetNbOfAnodes", this);
    setNbOfAnodesCmd->SetGuidance("Set the number of anodes.");
    setNbOfAnodesCmd->SetDefaultValue(64);
    setNbOfAnodesCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    setNameOfSimulationCmd = 
      new G4UIcmdWithAString("/DMPX/geometry/SetNameOfSimulation", this);
    setNameOfSimulationCmd->SetGuidance("Set the name of the simulation.");
    setNameOfSimulationCmd->SetDefaultValue("DMPX");
    setNameOfSimulationCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    setCathodes1_LengthXCmd = 
      new G4UIcmdWithADoubleAndUnit("/DMPX/geometry/cathodes1_LengthX", this); 
    setCathodes1_LengthXCmd->SetGuidance("Set the absolute length in X for the first cathodes.");
    setCathodes1_LengthXCmd->SetUnitCategory("Length");
    setCathodes1_LengthXCmd->SetDefaultValue(5.0 * mm); 
    setCathodes1_LengthXCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    setCathodes1_LengthYCmd = 
      new G4UIcmdWithADoubleAndUnit("/DMPX/geometry/cathodes1_LengthY", this); 
    setCathodes1_LengthYCmd->SetGuidance("Set the absolute length in Y for the first cathodes."); 
    setCathodes1_LengthYCmd->SetUnitCategory("Length");
    setCathodes1_LengthYCmd->SetDefaultValue(5.0 * mm); 
    setCathodes1_LengthYCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    setCathodes1_LengthZCmd = 
      new G4UIcmdWithADoubleAndUnit("/DMPX/geometry/cathodes1_LengthZ", this); 
    setCathodes1_LengthZCmd->SetGuidance("Set the absolute length in Z for the first cathodes."); 
    setCathodes1_LengthZCmd->SetUnitCategory("Length");
    setCathodes1_LengthZCmd->SetDefaultValue(5.0 * mm); 
    setCathodes1_LengthZCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    setCathodes1_XPosCmd = 
      new G4UIcmdWithADoubleAndUnit("/DMPX/geometry/cathodes1_XPos", this); 
    setCathodes1_XPosCmd->SetGuidance("Set the X center position for the first cathodes."); 
    setCathodes1_XPosCmd->SetUnitCategory("Length");
    setCathodes1_XPosCmd->SetDefaultValue(5.0 * mm); 
    setCathodes1_XPosCmd->AvailableForStates(G4State_PreInit, G4State_Idle);


    setCathodes1_ZPosCmd = 
      new G4UIcmdWithADoubleAndUnit("/DMPX/geometry/cathodes1_ZPos", this); 
    setCathodes1_ZPosCmd->SetGuidance("Set the Z center position for the first cathodes."); 
    setCathodes1_ZPosCmd->SetUnitCategory("Length");
    setCathodes1_ZPosCmd->SetDefaultValue(5.0 * mm); 
    setCathodes1_ZPosCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    

    // Default values
    worldHalfLength = 0.2 * m; // Default world half length
    checkOverlaps = false; // Default not to check overlaps
    pressure = 0.0 * bar; 
    gasBoxLengthX = 0.0 * mm;
    gasBoxLengthY = 0.0 * mm;
    gasBoxLengthZ = 0.0 * mm; 
    gasBoxCenterPositionX = 0.0 * mm;
    gasBoxCenterPositionY = 0.0 * mm;
    gasBoxCenterPositionZ = 0.0 * mm;
    kryptonPercentage = 90.0;
    ch4Percentage = 10.0;
    anodesHalfLength = 16.0 * mm;
    anodesR = 0.1 * mm;
    anodesSpacing = 2.0 * mm; 
    nbOfAnodes = 64;
    nameOfSimulation = "DMPX";
    cathodes1_LengthX = 5.0 * mm; 
    cathodes1_LengthY = 5.0 * mm; 
    cathodes1_LengthZ = 5.0 * mm; 
    cathodes1_XPos = 5.0 * mm; 
    cathodes1_ZPos = 5.0 * mm; 
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorMessenger::~DetectorMessenger() {
    delete miniDir;
    delete geometryDir;
    delete setWorldHalfLengthCmd;
    delete setCheckOverlapsCmd;
    delete setGasPressCmd;
    delete setGasBoxLengthXCmd;
    delete setGasBoxLengthYCmd;
    delete setGasBoxLengthZCmd;
    delete setGasBoxCenterPositionXCmd;
    delete setGasBoxCenterPositionYCmd;
    delete setGasBoxCenterPositionZCmd;
    delete setKryptonPercentageCmd;
    delete setCH4PercentageCmd;
    delete setAnodesHalfLengthCmd;
    delete setAnodesRCmd;
    delete setAnodesSpacingCmd;
    delete setNbOfAnodesCmd;
    delete setNameOfSimulationCmd;
    delete setCathodes1_LengthXCmd; 
    delete setCathodes1_LengthYCmd; 
    delete setCathodes1_LengthZCmd; 
    delete setCathodes1_XPosCmd; 
    delete setCathodes1_ZPosCmd; 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorMessenger::SetNewValue(G4UIcommand* command, G4String newValues) {
  if (command == setWorldHalfLengthCmd) {
    worldHalfLength = setWorldHalfLengthCmd->GetNewDoubleValue(newValues);
    detector->SetWorldHalfLength(worldHalfLength);
    G4cout << "(Debug: DetectorMessenger.cc) Setting world half length to " << 
    G4BestUnit(worldHalfLength, "Length") << G4endl;
  } else if (command == setCheckOverlapsCmd) {
    checkOverlaps = setCheckOverlapsCmd->GetNewBoolValue(newValues);
    detector->CheckOverlaps(checkOverlaps);
    G4cout << "(Debug: DetectorMessenger.cc) Setting check overlaps to " << 
    (checkOverlaps ? "true" : "false") << G4endl;
  } else if (command == setGasPressCmd) {
    pressure = setGasPressCmd->GetNewDoubleValue(newValues);
    detector->SetGasPressure(pressure);
    G4cout << "(Debug: DetectorMessenger.cc) Setting gas pressure to " << 
    G4BestUnit(pressure, "Pressure") << G4endl;
  } else if (command == setKryptonPercentageCmd) {
    kryptonPercentage = setKryptonPercentageCmd->GetNewDoubleValue(newValues);
    detector->SetKryptonPercentage(kryptonPercentage);
    G4cout << "(Debug: DetectorMessenger.cc) Setting Krypton percentage to " << 
    kryptonPercentage << "%" << G4endl;
  } else if (command == setCH4PercentageCmd) {
    ch4Percentage = setCH4PercentageCmd->GetNewDoubleValue(newValues);
    detector->SetCH4Percentage(ch4Percentage);
    G4cout << "(Debug: DetectorMessenger.cc) Setting CH4 percentage to " << 
    ch4Percentage << "%" << G4endl;
  } else if (command == setGasBoxLengthXCmd) {
    gasBoxLengthX = setGasBoxLengthXCmd->GetNewDoubleValue(newValues);
    detector->SetGasBoxLengthX(gasBoxLengthX);
    G4cout << "(Debug: DetectorMessenger.cc) Setting gas box length in X to " << 
    G4BestUnit(gasBoxLengthX, "Length") << G4endl;
  } else if (command == setGasBoxLengthYCmd) {
    gasBoxLengthY = setGasBoxLengthYCmd->GetNewDoubleValue(newValues);
    detector->SetGasBoxLengthY(gasBoxLengthY);
    G4cout << "(Debug: DetectorMessenger.cc) Setting gas box length in Y to " << 
    G4BestUnit(gasBoxLengthY, "Length") << G4endl;
  } else if (command == setGasBoxLengthZCmd) {
    gasBoxLengthZ = setGasBoxLengthZCmd->GetNewDoubleValue(newValues);
    detector->SetGasBoxLengthZ(gasBoxLengthZ);
    G4cout << "(Debug: DetectorMessenger.cc) Setting gas box length in Z to " << 
    G4BestUnit(gasBoxLengthZ, "Length") << G4endl;
  } else if (command == setGasBoxCenterPositionXCmd) {
    gasBoxCenterPositionX = setGasBoxCenterPositionXCmd->GetNewDoubleValue(newValues);
    detector->SetGasBoxCenterPositionX(gasBoxCenterPositionX);
    G4cout << "(Debug: DetectorMessenger.cc) Setting gas box center position in X to " << 
    G4BestUnit(gasBoxCenterPositionX, "Length") << G4endl;
  } else if (command == setGasBoxCenterPositionYCmd) {
    gasBoxCenterPositionY = setGasBoxCenterPositionYCmd->GetNewDoubleValue(newValues);
    detector->SetGasBoxCenterPositionY(gasBoxCenterPositionY);
    G4cout << "(Debug: DetectorMessenger.cc) Setting gas box center position in Y to " << 
    G4BestUnit(gasBoxCenterPositionY, "Length") << G4endl;
  } else if (command == setGasBoxCenterPositionZCmd) {
    gasBoxCenterPositionZ = setGasBoxCenterPositionZCmd->GetNewDoubleValue(newValues);
    detector->SetGasBoxCenterPositionZ(gasBoxCenterPositionZ);
    G4cout << "(Debug: DetectorMessenger.cc) Setting gas box center position in Z to " << 
    G4BestUnit(gasBoxCenterPositionZ, "Length") << G4endl;
  } else if (command == setAnodesHalfLengthCmd) {
    anodesHalfLength = setAnodesHalfLengthCmd->GetNewDoubleValue(newValues);
    detector->SetAnodesHalfLength(anodesHalfLength);
    G4cout << "(Debug: DetectorMessenger.cc) Setting anodes half length to " << 
    G4BestUnit(anodesHalfLength, "Length") << G4endl;
  } else if (command == setAnodesRCmd) {
    anodesR = setAnodesRCmd->GetNewDoubleValue(newValues);
    detector->SetAnodesR(anodesR);
    G4cout << "(Debug: DetectorMessenger.cc) Setting anodes radius to " << 
    G4BestUnit(anodesR, "Length") << G4endl;
  } else if (command == setAnodesSpacingCmd) {
    anodesSpacing = setAnodesSpacingCmd->GetNewDoubleValue(newValues);
    detector->SetAnodesSpacing(anodesSpacing);
    G4cout << "(Debug: DetectorMessenger.cc) Setting anodes spacing to " << 
    G4BestUnit(anodesSpacing, "Length") << G4endl;
  } else if (command == setNbOfAnodesCmd) {
    nbOfAnodes = setNbOfAnodesCmd->GetNewIntValue(newValues);
    detector->SetNbOfAnodes(nbOfAnodes);
    G4cout << "(Debug: DetectorMessenger.cc) Setting number of anodes to " << 
    nbOfAnodes << G4endl;
  } else if (command == setNameOfSimulationCmd) {
    nameOfSimulation = newValues;
    detector->SetNameOfSimulation(nameOfSimulation);
    G4cout << "(Debug: DetectorMessenger.cc) Setting name of simulation to " << 
    nameOfSimulation << G4endl;
  } else if (command == setCathodes1_LengthXCmd) {
    cathodes1_LengthX = setCathodes1_LengthXCmd->GetNewDoubleValue(newValues);
    detector->SetCathodes1_LengthX(cathodes1_LengthX);
    G4cout << "(Debug: DetectorMessenger.cc) Setting the length in X for the first cathodes to: " << 
    G4BestUnit(cathodes1_LengthX, "Length") << G4endl;
  } else if (command == setCathodes1_LengthYCmd) {
    cathodes1_LengthY = setCathodes1_LengthYCmd->GetNewDoubleValue(newValues);
    detector->SetCathodes1_LengthY(cathodes1_LengthY);
    G4cout << "(Debug: DetectorMessenger.cc) Setting the length in Y for the first cathodes to: " << 
    G4BestUnit(cathodes1_LengthY, "Length") << G4endl;
  } else if (command == setCathodes1_LengthZCmd) {
    cathodes1_LengthZ = setCathodes1_LengthZCmd->GetNewDoubleValue(newValues);
    detector->SetCathodes1_LengthZ(cathodes1_LengthZ);
    G4cout << "(Debug: DetectorMessenger.cc) Setting the length in Z for the first cathodes to: " << 
    G4BestUnit(cathodes1_LengthZ, "Length") << G4endl; 
  } else if (command == setCathodes1_XPosCmd) {
    cathodes1_XPos = setCathodes1_XPosCmd->GetNewDoubleValue(newValues);
    detector->SetCathodes1_XPos(cathodes1_XPos);
    G4cout << "(Debug: DetectorMessenger.cc) Setting the center position in X for the first cathodes to: " << 
    G4BestUnit(cathodes1_XPos, "Length") << G4endl;
  } else if (command == setCathodes1_ZPosCmd) {
    cathodes1_ZPos = setCathodes1_ZPosCmd->GetNewDoubleValue(newValues);
    detector->SetCathodes1_ZPos(cathodes1_ZPos);
    G4cout << "(Debug: DetectorMessenger.cc) Setting the center position in Z for the first cathodes to: " << 
    G4BestUnit(cathodes1_ZPos, "Length") << G4endl;
  } else {
    G4cerr << "(Error: DetectorMessenger.cc) Command not recognized!" << G4endl;
  }

}
