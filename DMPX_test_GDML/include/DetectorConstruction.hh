#ifndef DetectorConstruction_hh
#define DetectorConstruction_hh 1

#include "HeedDeltaElectronModel.hh"
#include "DetectorMessenger.hh"
#include "GasModelParameters.hh"

#include "G4VUserDetectorConstruction.hh"
#include "G4SystemOfUnits.hh"
#include "G4UserLimits.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Sphere.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"
#include "G4RunManager.hh"
#include "G4FieldManager.hh"
#include "G4PhysicalConstants.hh"
#include "G4UniformElectricField.hh"
#include "G4EqMagElectricField.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4Polycone.hh"
#include "G4Polyhedra.hh"
#include "G4UnionSolid.hh"
#include "G4Region.hh"
#include "G4Orb.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class G4VSolid;
class G4LogicalVolume;
class G4VPhysicalVolume;
class G4UniformElectricField;

/*! \class  DetectorConstruction*/
/*! \brief class derived from G4VUserDetectorConstruction*/

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class DetectorConstruction : public G4VUserDetectorConstruction {
 public:
  // Initializing an instance of the DetectorConstruction class
  DetectorConstruction(GasModelParameters*);
  
  // Destructor of the DetectorConstruction class
  virtual ~DetectorConstruction();

  /* 
  What is a virtual method? Example:
  class Base {
  public:
      virtual void Display() {
          std::cout << "Base class display" << std::endl;
      }
  };

  class Derived : public Base {
  public:
      void Display() override { // Overrides the base class function
          std::cout << "Derived class display" << std::endl;
      }
  };

  Base* obj = new Derived();
  obj->Display(); // Outputs: "Derived class display"
  */

  // Mandatory methods
  virtual G4VPhysicalVolume* Construct();
  virtual void ConstructSDandField();

  //Setters for the dimensions and environment variables of the setup
  inline void CheckOverlaps(G4bool co){checkOverlaps=co;};
  inline void SetWorldHalfLength(G4double d){worldHalfLength=d;};
  inline void SetGasPressure(G4double d){gasPressure=d;};
  inline void SetTemperature(G4double d){temperature=d;};
  inline void SetGasBoxLengthX(G4double d){GasBoxLengthX=d;};
  inline void SetGasBoxLengthY(G4double d){GasBoxLengthY=d;};
  inline void SetGasBoxLengthZ(G4double d){GasBoxLengthZ=d;};
  inline void SetGasBoxCenterPositionX(G4double d){GasBoxCenterPositionX=d;};
  inline void SetGasBoxCenterPositionY(G4double d){GasBoxCenterPositionY=d;};
  inline void SetGasBoxCenterPositionZ(G4double d){GasBoxCenterPositionZ=d;};
  //Getters for the dimensions and environment variables of the setup
  inline G4double GetWorldHalfLength(){return worldHalfLength;};
  inline G4double GetGasPressure(){return gasPressure;};
  inline G4double GetTemperature(){return temperature;};
  inline G4double GetKryptonPercentage(){return kryptonPercentage;};
  inline G4double GetCH4Percentage(){return ch4Percentage;}; 
  inline G4double GetGasBoxLengthX(){return GasBoxLengthX;};
  inline G4double GetGasBoxLengthY(){return GasBoxLengthY;};
  inline G4double GetGasBoxLengthZ(){return GasBoxLengthZ;};
  inline G4double GetGasBoxCenterPositionX(){return GasBoxCenterPositionX;};
  inline G4double GetGasBoxCenterPositionY(){return GasBoxCenterPositionY;};
  inline G4double GetGasBoxCenterPositionZ(){return GasBoxCenterPositionZ;};

  /*
  What is an inline function? Example:
  inline int Add(int a, int b) {
      return a + b;
  }

  int result = Add(3, 5); // The compiler may replace this call with "int result = 3 + 5;"
  */
    
 // Variables only accessible by the class itself or related classes
 private:
  DetectorMessenger* detectorMessenger;
  G4LogicalVolume* logicGasBox;
  GasModelParameters* fGasModelParameters;
  G4bool checkOverlaps; // Check overlaps in the detector geometry if true
  G4double worldHalfLength; //World volume is a cube with side length = 2m;
  G4double gasPressure; // pressure in the gas
  G4double temperature; // temperature of the gas
  G4double kryptonPercentage;
  G4double ch4Percentage;
  G4double GasBoxLengthX; // Length of the gas box in the X direction   
  G4double GasBoxLengthY; // Length of the gas box in the Y direction
  G4double GasBoxLengthZ; // Length of the gas box in the Z direction
  G4double GasBoxCenterPositionX; // X position of the gas box center
  G4double GasBoxCenterPositionY; // Y position of the gas box center 
  G4double GasBoxCenterPositionZ; // Z position of the gas box center
  G4UniformElectricField* pEMfield; // Pointer to the electric field
  G4EqMagElectricField* pEquation;  // Pointer to the equation of motion
  G4ChordFinder* pChordFinder;      // Pointer to the chord finder
};

#endif // DetectorConstruction_hh
