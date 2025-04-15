#include "G4GDMLParser.hh"
#include "DetectorConstruction.hh"
#include "G4PVParameterised.hh"
#include "G4PVReplica.hh"
#include "G4RotationMatrix.hh"
#include "G4UnitsTable.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4OpticalSurface.hh"
#include "G4Trd.hh"
#include "G4Threading.hh"
#include "G4RegionStore.hh"
#include "G4UniformMagField.hh"
#include "G4FieldManager.hh"
#include "G4Cons.hh"
#include "G4IntersectionSolid.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Trd.hh"
#include "DetectorMessenger.hh"
#include "GasBoxSD.hh"
#include "SiliconSD.hh"
#include "HeedDeltaElectronModel.hh"
#include "HeedNewTrackModel.hh"
#include "G4SDManager.hh"



DetectorConstruction::DetectorConstruction(GasModelParameters* gmp)
    :
    fGasModelParameters(gmp),
    checkOverlaps(0),
    worldHalfLength(2.*m), //World volume is a cube with side length = 3m;
    gasPressure(1.*bar), // Pressure inside the gas
    temperature(273.15*kelvin), // temperature
    kryptonPercentage(90.0), // mixture settings
    ch4Percentage(10.0)
{
  detectorMessenger = new DetectorMessenger(this);
  G4GDMLParser parser; 
  parser.SetOverlapCheck(false);
  parser.Read("../World.gdml", false);
  G4VPhysicalVolume* worldPhys = parser.GetWorldVolume();

  G4cout << "Loaded " 
       << G4LogicalVolumeStore::GetInstance()->size() 
       << " logical volumes from GDML." << G4endl;


}

DetectorConstruction::~DetectorConstruction() {
  delete detectorMessenger;
}

G4VPhysicalVolume* DetectorConstruction::Construct(){
  /* The World volume is a vacuum in which a gastube is placed with the walls made out of Aluminum. The
  endcaps are Silicon detectors, used as calorimeter 
  */
    
  //Colors for visualization
  G4VisAttributes* red = new G4VisAttributes(G4Colour(1., 0., 0.));
  G4VisAttributes* green = new G4VisAttributes(G4Colour(0., 1., 0.));
  G4VisAttributes* blue = new G4VisAttributes(G4Colour(0., 0., 1.));
  G4VisAttributes* yellow = new G4VisAttributes(G4Colour(1.0, 1.0, 0.));
  G4VisAttributes* purple = new G4VisAttributes(G4Colour(1.0, 0., 1.0));

  /*First: build materials
    World: vacuum
    Gas: mixture of Kr and CH4
  */

  //World material: vacuum
  G4NistManager* man = G4NistManager::Instance();
  man->SetVerbose(1);
  G4Material* vacuum = man->FindOrBuildMaterial("G4_Galactic");

  G4Element* elC = man->FindOrBuildElement("C");
  G4Element* elH = man->FindOrBuildElement("H");
  
  
  //Gas material: mixture of Kr or CH4
  G4double nMoles = gasPressure / (8.314 * joule / mole * temperature);
  G4Material* mixture=NULL;
  G4VPhysicalVolume* physiWorld = NULL;

  // TPC setup
  gasboxR = 1*m;
  gasboxH = 1*m;
  
  G4double molarMass = 83.798*g/mole;  // pure krypton
  
  G4double gasDensityKr = nMoles * molarMass;
  G4cout << "gasPressure: " << G4BestUnit(gasPressure, "Pressure") << G4endl;

  G4cout << "gasDensityNe: " << G4BestUnit(gasDensityKr, "Volumic Mass") << G4endl;

  G4Material* krypton = new G4Material("krypton", 10, molarMass, gasDensityKr,
                                      kStateGas, temperature, gasPressure);
  G4double molfracKr = (kryptonPercentage/100.) * molarMass;

  // CH4 Density 0.657 mg/mL
  molarMass = 16.04206*g/mole;  // source wikipedia
  G4double gasDensityCH4 = nMoles * molarMass;
  G4cout << "Gas density CH4: " << G4BestUnit(gasDensityCH4, "Volumic Mass") << G4endl;
  G4Material* CH4 = new G4Material("ch4", gasDensityCH4, 2,
                                    kStateGas, temperature, gasPressure);
  CH4->AddElement(elC, 1);
  CH4->AddElement(elH, 4);
  
  G4double molfracCH4 = (ch4Percentage/100.)*molarMass;
  
 
  G4double molfracKr_norm = molfracKr/(molfracKr+molfracCH4);
  G4double molfracCH4_norm = 1 - molfracKr_norm;

  G4cout << "Molar fraction Kr: " << molfracKr_norm << G4endl;
  G4cout << "Molar fraction CH4: " << molfracCH4_norm << G4endl;

  G4double gasDensityMixture = (kryptonPercentage/100.) * gasDensityKr +
                               ch4Percentage/100. * gasDensityCH4;
  
  mixture = new G4Material("mixture", gasDensityMixture, 2);
  
  
  mixture->AddMaterial(krypton, molfracKr_norm);
  mixture->AddMaterial(CH4, molfracCH4_norm);
  G4cout << "Gas density Kr + CH4: " << G4BestUnit(gasDensityMixture, "Volumic Mass") << G4endl;
    
  
  //World Volume
  G4Box* solidWorld = new G4Box("solidWorld_box", worldHalfLength, worldHalfLength, worldHalfLength);
  G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, vacuum, "solidWorld_log");
  
  physiWorld = new G4PVPlacement(0, G4ThreeVector(), logicWorld, "solidWorld_phys", 0, false, 0, checkOverlaps);
  logicWorld->SetVisAttributes(& G4VisAttributes::GetInvisible());
  
  //GasBox volume
  G4RotationMatrix* myRotation = new G4RotationMatrix();
  myRotation->rotateX(90.*deg);
  myRotation->rotateY(0.*deg);
  myRotation->rotateZ(0.*rad);
  G4Tubs* solidGasBox = new G4Tubs("solid_gasbox_tube",0,gasboxR,gasboxH*0.5, 0., twopi);
  logicGasBox = new G4LogicalVolume(solidGasBox, mixture, "solidGasBox_log");
  new G4PVPlacement(myRotation,G4ThreeVector(), logicGasBox,"solidGasBox_phys",logicWorld,false,0,checkOverlaps);
  
  
  //Construct a G4Region, connected to the logical volume in which you want to use the G4FastSimulationModel
  G4Region* regionGas = new G4Region("GasRegion");
  regionGas->AddRootLogicalVolume(logicGasBox);
    
  return physiWorld;

}

void DetectorConstruction::ConstructSDandField(){
  G4SDManager* SDManager = G4SDManager::GetSDMpointer();
  G4String GasBoxSDname = "interface/GasBoxSD";
  GasBoxSD* myGasBoxSD = new GasBoxSD(GasBoxSDname);
  SDManager->SetVerboseLevel(1);
  SDManager->AddNewDetector(myGasBoxSD);
  SetSensitiveDetector(logicGasBox,myGasBoxSD);

  G4String SiliconSDname = "interface/SiliconSD";
  SiliconSD* mySiliconSD = new SiliconSD(SiliconSDname);
  SDManager->SetVerboseLevel(1);
  SDManager->AddNewDetector(mySiliconSD);
  SetSensitiveDetector(logicCalo,mySiliconSD);

  //These commands generate the four gas models and connect it to the GasRegion
  G4Region* region = G4RegionStore::GetInstance()->GetRegion("GasRegion");
  new HeedNewTrackModel(fGasModelParameters,"HeedNewTrackModel",region,this,myGasBoxSD);
  new HeedDeltaElectronModel(fGasModelParameters,"HeedDeltaElectronModel",region,this,myGasBoxSD);
}

