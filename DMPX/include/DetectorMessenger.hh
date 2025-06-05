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

class DetectorMessenger : public G4UImessenger {
	public:
		DetectorMessenger(DetectorConstruction*);
		~DetectorMessenger();
		void SetNewValue(G4UIcommand*, G4String);

		// Other parameters
		G4String GetNameOfSimulation() const { return nameOfSimulation; }
		G4double GetWorldHalfLength() const { return worldHalfLength; }
		G4bool GetCheckOverlaps() const { return checkOverlaps; }
		G4double GetPressure() const { return pressure; } 

		// Gas percentages
		G4double GetKryptonPercentage() const { return kryptonPercentage; }
		G4double GetCH4Percentage() const { return ch4Percentage; }

		// Gas box
		G4double GetGasBoxLengthX() const { return gasBoxLengthX; }
		G4double GetGasBoxLengthY() const { return gasBoxLengthY; }
		G4double GetGasBoxLengthZ() const { return gasBoxLengthZ; }
		G4double GetGasBoxCenterPositionX() const { return gasBoxCenterPositionX; }
		G4double GetGasBoxCenterPositionY() const { return gasBoxCenterPositionY; }
		G4double GetGasBoxCenterPositionZ() const { return gasBoxCenterPositionZ; }

		// Anodes and cathodes
		G4double GetAnodesHalfLength() const { return anodesHalfLength; }
		G4double GetAnodesR() const { return anodesR; }
		G4double GetAnodesSpacing() const { return anodesSpacing; }
		G4int GetNbOfAnodes() const { return nbOfAnodes; }
		G4double GetCathodes1_LengthX() const { return cathodes1_LengthX; } 
		G4double GetCathodes1_LengthY() const { return cathodes1_LengthY; } 
		G4double GetCathodes1_LengthZ() const { return cathodes1_LengthZ; } 
		G4double GetCathodes1_XPos() const { return cathodes1_XPos; }
		G4double GetCathodes1_ZPos() const { return cathodes1_ZPos; }

 	private:
		DetectorConstruction* detector;
		G4UIdirectory* miniDir;      // /DMPX/
		G4UIdirectory* geometryDir;  // /DMPX/geometry/
		
		// Other commands and variables for the detector
		G4UIcmdWithAString* setNameOfSimulationCmd;
		G4UIcmdWithADoubleAndUnit* setWorldHalfLengthCmd;
		G4UIcmdWithABool* setCheckOverlapsCmd;
		G4UIcmdWithADoubleAndUnit* setGasPressCmd;
		G4bool checkOverlaps; // Flag to check overlaps
		G4double worldHalfLength; 
		G4double pressure; 
		G4String nameOfSimulation;

		// Gas box commands and variables
		G4UIcmdWithADoubleAndUnit* setGasBoxLengthXCmd;
		G4UIcmdWithADoubleAndUnit* setGasBoxLengthYCmd;
		G4UIcmdWithADoubleAndUnit* setGasBoxLengthZCmd;
		G4UIcmdWithADoubleAndUnit* setGasBoxCenterPositionXCmd;
		G4UIcmdWithADoubleAndUnit* setGasBoxCenterPositionYCmd;
		G4UIcmdWithADoubleAndUnit* setGasBoxCenterPositionZCmd;
		G4double gasBoxLengthX; 
		G4double gasBoxLengthY;
		G4double gasBoxLengthZ;
		G4double gasBoxCenterPositionX;
		G4double gasBoxCenterPositionY;
		G4double gasBoxCenterPositionZ;

		// Gas percentages commands and variables
		G4UIcmdWithADouble* setKryptonPercentageCmd;
		G4UIcmdWithADouble* setCH4PercentageCmd;
		G4double kryptonPercentage;
		G4double ch4Percentage;

		// Anodes and cathodes commands and variables
		G4UIcmdWithADoubleAndUnit* setAnodesHalfLengthCmd;
		G4UIcmdWithADoubleAndUnit* setAnodesRCmd;
		G4UIcmdWithADoubleAndUnit* setAnodesSpacingCmd;
		G4UIcmdWithAnInteger* setNbOfAnodesCmd;
		G4UIcmdWithADoubleAndUnit* setCathodes1_LengthXCmd;
		G4UIcmdWithADoubleAndUnit* setCathodes1_LengthYCmd;
		G4UIcmdWithADoubleAndUnit* setCathodes1_LengthZCmd;
		G4UIcmdWithADoubleAndUnit* setCathodes1_XPosCmd; 
		G4UIcmdWithADoubleAndUnit* setCathodes1_ZPosCmd; 
		G4double anodesHalfLength;
		G4double anodesR;
		G4double anodesSpacing;
		G4int nbOfAnodes;
		G4double cathodes1_LengthX; 
		G4double cathodes1_LengthY; 
		G4double cathodes1_LengthZ; 
		G4double cathodes1_XPos; 
		G4double cathodes1_ZPos; 
};

#endif
