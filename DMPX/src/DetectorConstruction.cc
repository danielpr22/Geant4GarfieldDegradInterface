#include "../include/DetectorConstruction.hh"
#include "../include/DetectorMessenger.hh"
#include "../include/GasBoxSD.hh"
#include "../include/SiliconSD.hh"
#include "../include/HeedDeltaElectronModel.hh"
#include "../include/HeedNewTrackModel.hh"

#include "G4GDMLParser.hh"
#include "G4PVParameterised.hh"
#include "G4PVReplica.hh"
#include "G4RotationMatrix.hh"
#include "G4UnitsTable.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4OpticalSurface.hh"
#include "G4Trd.hh"
#include "G4Threading.hh"
#include "G4RegionStore.hh"
#include "G4UniformMagField.hh"
#include "G4FieldManager.hh"
#include "G4Cons.hh"
#include "G4IntersectionSolid.hh"
#include "G4Trd.hh"
#include "G4SDManager.hh"


DetectorConstruction::DetectorConstruction(GasModelParameters* gmp)
    :
    fGasModelParameters(gmp),
    checkOverlaps(0),
    worldHalfLength(1.*m), //World volume is a cube with side length = 3m;
    gasPressure(1.*bar), // Pressure inside the gas
    temperature(273.15*kelvin), // temperature
    kryptonPercentage(90), // mixture settings
    ch4Percentage(10)
{
  detectorMessenger = new DetectorMessenger(this);
}


DetectorConstruction::~DetectorConstruction() {
  delete detectorMessenger;
}

G4VPhysicalVolume* DetectorConstruction::Construct(){

    G4NistManager* man = G4NistManager::Instance();
    man->SetVerbose(1);

    // Defining the gas elements, He, Kr and CH4
    G4Material* Helium = man->FindOrBuildMaterial("G4_He");
    G4Material* Krypton = man->FindOrBuildMaterial("G4_Kr");
    G4Material* Methane = man->FindOrBuildMaterial("G4_METHANE");

    /* Calculating the mass fractions for the gas mixture:
    Kr: 83,798 g/mol
    CH4: 16.04 g/mol
    If we have 9 mol of Kr, we have 9*83.798 = 754.182 g of Kr
    If we have 1 mol of CH4, we have 1*16.04 = 16.04 g of CH4

    The mass fraction of Kr is 754.182/(754.182 + 16.04) = 0.973 
    The mass fraction of CH4 is 1 - 0.973 = 0.027

    The density of the mixture is calculated using the formula:
    density = (mass fraction of Kr * density of Kr) + (mass fraction of CH4 * density of CH4)
    The density of Kr is 0.00375 g/cm3 and the density of CH4 is 0.000716 g/cm3 (both at STP)
    The density of the mixture is (0.973*0.00375)+ (0.027*0.000716) = 0.00366 g/cm3
    */

    // Defining the gas mixture by fractional mass
    G4double density = 0.00366 * g/cm3;
    G4Material* KrCH4_90_10 = new G4Material("KrCH4_90_10"  , density, 2, kStateGas, 273.15*kelvin, 1.*atmosphere);
    KrCH4_90_10->AddMaterial(Krypton, 0.973); // 97.3% by mass (90% molar)
    KrCH4_90_10->AddMaterial(Methane, 0.027); // 2.7% by mass (10% molar)

    G4GDMLParser parser; 
    parser.SetOverlapCheck(true);
    parser.Read("../../DMPX/World.gdml", false);
    G4VPhysicalVolume* worldPhys = parser.GetWorldVolume();

    /*First: build materials
        First cylinder: He
        Second cylinder: Kr + CH4 at a certain flux
        Third cylinder: Kr + CH4 at a certain flux
        Gas: mixture of Kr and CH4
        Anodes: Silicon 
    */
    
    //Colors for visualization
    G4VisAttributes* red = new G4VisAttributes(G4Colour(1., 0., 0.));
    G4VisAttributes* green = new G4VisAttributes(G4Colour(0., 1., 0.));
    G4VisAttributes* blue = new G4VisAttributes(G4Colour(0., 0., 1.));
    G4VisAttributes* yellow = new G4VisAttributes(G4Colour(1.0, 1.0, 0.));
    G4VisAttributes* purple = new G4VisAttributes(G4Colour(1.0, 0., 1.0));

    // Dimensions of the Helium gas cylinder
    gasboxR = 0.04*m;
    gasboxH = 0.1*m;
    G4RotationMatrix* myRotation = new G4RotationMatrix();
    myRotation->rotateX(0.*deg);
    myRotation->rotateY(0.*deg);
    myRotation->rotateZ(0.*rad);
    G4Tubs* KrCH4GasBox = new G4Tubs("_gasbox_tube",0,gasboxR,gasboxH*0.5, 0., twopi);
    logicGasBox = new G4LogicalVolume(KrCH4GasBox, KrCH4_90_10, "solidGasBox");

    G4VisAttributes* gasVis = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0, 0.3)); // RGBA: Blue with 30% opacity
    gasVis->SetForceSolid(true);  // Makes sure the volume is drawn as a surface
    logicGasBox->SetVisAttributes(gasVis);

    // Get logical volume of the GDML world
    G4LogicalVolume* logicWorld = worldPhys->GetLogicalVolume();

    // Place your gas volume inside the GDML world
    new G4PVPlacement(
        myRotation,
        G4ThreeVector(0., 0., 100.), // Adjust position if needed
        logicGasBox,
        "physGasBox",
        logicWorld,
        false,
        0,
        true
    );

    G4Region* gasRegion = new G4Region("GasRegion");
    gasRegion->AddRootLogicalVolume(logicGasBox);

     return worldPhys; 
}

void DetectorConstruction::ConstructSDandField(){

  G4LogicalVolume* logicGasBox = G4LogicalVolumeStore::GetInstance()->GetVolume("solidGasBox");
  if (!logicGasBox) {
      G4cerr << "Error: Logical volume 'solidGasBox' not found!" << G4endl;
      return;
  }
  G4SDManager* SDManager = G4SDManager::GetSDMpointer();
  G4String GasBoxSDname = "interface/GasBoxSD";
  GasBoxSD* myGasBoxSD = new GasBoxSD(GasBoxSDname);
  SDManager->SetVerboseLevel(1);
  SDManager->AddNewDetector(myGasBoxSD);
  SetSensitiveDetector(logicGasBox,myGasBoxSD);


  // Attaching the volume of the detector to the class SiliconSD
  G4String SiliconSDname = "interface/SiliconSD";
  SiliconSD* mySiliconSD = new SiliconSD(SiliconSDname);
  G4SDManager::GetSDMpointer()->AddNewDetector(mySiliconSD);
  
  G4LogicalVolume* logicDetector = G4LogicalVolumeStore::GetInstance()->GetVolume("__vol__11_");
  SetSensitiveDetector(logicDetector, mySiliconSD);

  //These commands generate the four gas models and connect it to the GasRegion
  G4Region* GasRegion = G4RegionStore::GetInstance()->GetRegion("GasRegion");
  new HeedNewTrackModel(fGasModelParameters,"HeedNewTrackModel",GasRegion,this,myGasBoxSD);
  new HeedDeltaElectronModel(fGasModelParameters,"HeedDeltaElectronModel",GasRegion,this,myGasBoxSD);
}

