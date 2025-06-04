#ifndef DetectorMessenger_h
#define DetectorMessenger_h 1

// Included from the loaded libraries (G4, ROOT, Garfield++, Degrad...)
#include "G4SystemOfUnits.hh"
#include "G4UImessenger.hh"

class DetectorConstruction;
class G4UIcommand;
class G4UIdirectory;
class G4UIcmdWithAString;
class G4UIcmdWithAnInteger;
class G4UIcmdWithABool;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithADouble;
class G4UIcmdWithoutParameter;
class G4UIcmdWith3Vector;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
/*! \class DetectorMessenger*/
/*! class derived from G4UImessenger*/
/*! List of available commands*/
/*!/DMPX/geometry/SetGasPressure*/
/*!/DMPX/geometry/SetNrUpperPlanes*/
/*!/DMPX/geometry/SetMaxStep*/
/*!/DMPX/geometry/SetBField*/
/*!/DMPX/geometry/EMField_version */
/*!/DMPX/geometry/ConstructWires */
/*!/DMPX/geometry/ConstructSlits5Vertical */
/*!/DMPX/geometry/ConstructSlits3Vertical */
/*!/DMPX/geometry/ConstructSlitVertical */
/*!/DMPX/geometry/ConstructSlitHorizontal */
/*!/DMPX/geometry/buildCells*/
/*!/DMPX/geometry/update */

class DetectorMessenger : public G4UImessenger {
 public:
  DetectorMessenger(DetectorConstruction*);
  ~DetectorMessenger();

  void SetNewValue(G4UIcommand*, G4String);
  G4double GetWorldHalfLength() const { return worldHalfLength; }
  G4bool GetCheckOverlaps() const { return checkOverlaps; }
  G4double GetPressure() const { return pressure; } 
  G4double GetKryptonPercentage() const { return kryptonPercentage; }
  G4double GetCH4Percentage() const { return ch4Percentage; }
  G4double GetGasBoxLengthX() const { return gasBoxLengthX; }
  G4double GetGasBoxLengthY() const { return gasBoxLengthY; }
  G4double GetGasBoxLengthZ() const { return gasBoxLengthZ; }
  G4double GetGasBoxCenterPositionX() const { return gasBoxCenterPositionX; }
  G4double GetGasBoxCenterPositionY() const { return gasBoxCenterPositionY; }
  G4double GetGasBoxCenterPositionZ() const { return gasBoxCenterPositionZ; }
  G4double GetAnodesHalfLength() const { return anodesHalfLength; }
  G4double GetAnodesR() const { return anodesR; }
  G4double GetAnodesSpacing() const { return anodesSpacing; }
  G4int GetNbOfAnodes() const { return nbOfAnodes; }
  G4String GetNameOfSimulation() const { return nameOfSimulation; }
  G4double GetCathodes1_LengthX() const { return cathodes1_LengthX; } 
  G4double GetCathodes1_LengthY() const { return cathodes1_LengthY; } 
  G4double GetCathodes1_LengthZ() const { return cathodes1_LengthZ; } 
  G4double GetCathodes1_XPos() const { return cathodes1_XPos; }
  G4double GetCathodes1_ZPos() const { return cathodes1_ZPos; }


 private:
  DetectorConstruction* detector;
  G4UIdirectory* miniDir;      // DMPX/
  G4UIdirectory* geometryDir;  // /DMPX/geometry/
  G4UIcmdWithADoubleAndUnit* setWorldHalfLengthCmd;
  G4UIcmdWithABool* setCheckOverlapsCmd;
  G4UIcmdWithADoubleAndUnit* setGasPressCmd;
  G4UIcmdWithADoubleAndUnit* setGasBoxLengthXCmd;
  G4UIcmdWithADoubleAndUnit* setGasBoxLengthYCmd;
  G4UIcmdWithADoubleAndUnit* setGasBoxLengthZCmd;
  G4UIcmdWithADoubleAndUnit* setGasBoxCenterPositionXCmd;
  G4UIcmdWithADoubleAndUnit* setGasBoxCenterPositionYCmd;
  G4UIcmdWithADoubleAndUnit* setGasBoxCenterPositionZCmd;
  G4UIcmdWithAString* setupNameCmd;
  G4UIcmdWithADouble* setKryptonPercentageCmd;
  G4UIcmdWithADouble* setCH4PercentageCmd;
  G4UIcmdWithADoubleAndUnit* setAnodesHalfLengthCmd;
  G4UIcmdWithADoubleAndUnit* setAnodesRCmd;
  G4UIcmdWithADoubleAndUnit* setAnodesSpacingCmd;
  G4UIcmdWithAnInteger* setNbOfAnodesCmd;
  G4UIcmdWithAString* setNameOfSimulationCmd;
  G4UIcmdWithADoubleAndUnit* setCathodes1_LengthXCmd;
  G4UIcmdWithADoubleAndUnit* setCathodes1_LengthYCmd;
  G4UIcmdWithADoubleAndUnit* setCathodes1_LengthZCmd;
  G4UIcmdWithADoubleAndUnit* setCathodes1_XPosCmd; 
  G4UIcmdWithADoubleAndUnit* setCathodes1_ZPosCmd; 


  G4bool checkOverlaps; // Flag to check overlaps
  G4double worldHalfLength; 
  G4double pressure; 
  G4double kryptonPercentage;
  G4double ch4Percentage;
  G4double gasBoxLengthX; 
  G4double gasBoxLengthY;
  G4double gasBoxLengthZ;
  G4double gasBoxCenterPositionX;
  G4double gasBoxCenterPositionY;
  G4double gasBoxCenterPositionZ;
  G4double anodesHalfLength;
  G4double anodesR;
  G4double anodesSpacing;
  G4int nbOfAnodes;
  G4String nameOfSimulation;
  G4double cathodes1_LengthX; 
  G4double cathodes1_LengthY; 
  G4double cathodes1_LengthZ; 
  G4double cathodes1_XPos; 
  G4double cathodes1_ZPos; 
};

#endif
