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
    worldHalfLength(3.*m), //World volume is a cube with side length = 3m;
    wallThickness(0.05*m), //thickness of the aluminum walls
    caloThickness(1.*mm), // thickness of the silicon detectors
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

    // Defining the gas mixture by fractional mass
    G4double density = 0.00344 * g/cm3;
    G4Material* KrCH4_90_10 = new G4Material("KrCH4_90_10"  , density, 2, kStateGas, 273.15*kelvin, 1.*atmosphere);
    KrCH4_90_10->AddMaterial(Krypton, 0.9792); // 97.92% by mass (90% in volume)
    KrCH4_90_10->AddMaterial(Methane, 0.0208); // 2.08% by mass (10% in volume)


    G4GDMLParser parser; 
    parser.SetOverlapCheck(false);
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
    gasboxR = 0.4*m;
    gasboxH = 0.3*m;
    G4RotationMatrix* myRotation = new G4RotationMatrix();
    myRotation->rotateX(90.*deg);
    myRotation->rotateY(0.*deg);
    myRotation->rotateZ(0.*rad);
    G4Tubs* HeliumGasBox = new G4Tubs("_gasbox_tube",0,gasboxR,gasboxH*0.5, 0., twopi);
    logicGasBox =   new G4LogicalVolume(HeliumGasBox, Helium, "solidGasBox_log");

    G4VisAttributes* gasVis = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0, 0.3)); // RGBA: Blue with 30% opacity
    gasVis->SetForceSolid(true);  // Makes sure the volume is drawn as a surface
    logicGasBox->SetVisAttributes(gasVis);

    // Get logical volume of the GDML world
    G4LogicalVolume* logicWorld = worldPhys->GetLogicalVolume();

    // Place your gas volume inside the GDML world
    new G4PVPlacement(
        myRotation,
        G4ThreeVector(0., 0., 0.),  // Adjust position if needed
        logicGasBox,
        "physGasBox",
        logicWorld,
        false,
        0,
        true
    );


  
//   //World Volume
    //  G4Box* solidWorld = new G4Box("solidWorld_box", worldHalfLength, worldHalfLength, worldHalfLength);
    //  G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, vacuum, "solidWorld_log");
  
//   physiWorld = new G4PVPlacement(0, G4ThreeVector(), logicWorld,
//                                                     "solidWorld_phys", 0, false, 0, checkOverlaps);
//   logicWorld->SetVisAttributes(& G4VisAttributes::GetInvisible());
  
//   //GasBox volume
    
  
//   //Silicon calorimeters
//   G4Tubs* solidCalo = new G4Tubs("solid_tube_Calo",gasboxR,gasboxR+caloThickness,gasboxH*0.5, 0., twopi);
//   logicCalo =
//   new G4LogicalVolume(solidCalo, siliconMaterial, "solidCalo_log");
//   new G4PVPlacement(myRotation,G4ThreeVector(), logicCalo,"solidCalo_phys",logicWorld,false,0,checkOverlaps);
  
 

     return worldPhys; 
    
//   return physiWorld;

}

void DetectorConstruction::ConstructSDandField(){

  G4LogicalVolume* logicGasBox = G4LogicalVolumeStore::GetInstance()->GetVolume("__phys_0");
  if (!logicGasBox) {
      G4cerr << "Error: Logical volume 'GasBox' not found!" << G4endl;
      return;
  }
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

