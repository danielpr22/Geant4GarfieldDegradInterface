#ifndef DetectorMessenger_h
#define DetectorMessenger_h 1

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
/*!/DMPX/geometry/BuildUpperScint*/
/*!/DMPX/geometry/BuildLowerScint*/
/*!/DMPX/geometry/update */

// The DetectorMessenger is a class that inherits from the Geant4 class G4UImessenger
class DetectorMessenger : public G4UImessenger {
 public:
  DetectorMessenger(DetectorConstruction*);

  // This is the destructor of the class, that cleans up any allocated ressources for the class.
  ~DetectorMessenger();

  void SetNewValue(G4UIcommand*, G4String);

 private:
  DetectorConstruction* detector;

  G4UIdirectory* miniDir;      ///<\brief /DMPX/
  G4UIdirectory* geometryDir;  ///<\brief /DMPX/geometry/

  G4UIcmdWithADoubleAndUnit* setGasPressCmd;
  G4UIcmdWithAString* setupNameCmd;
    
    
    
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
