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
#include "G4Threading.hh"
#include "G4RegionStore.hh"
#include "G4UniformElectricField.hh"
#include "G4FieldManager.hh"
#include "G4Cons.hh"
#include "G4IntersectionSolid.hh"
#include "G4Trd.hh"
#include "G4SDManager.hh"

// For the definition of the electric field
#include "G4EqMagElectricField.hh"
#include "G4UniformElectricField.hh"
#include "G4DormandPrince745.hh"
#include "G4ChordFinder.hh"


DetectorConstruction::DetectorConstruction(GasModelParameters* gmp)
    :
    fGasModelParameters(gmp),
    checkOverlaps(1),
    worldHalfLength(0.2*m),        // World volume is a cube with side length = 3m;
    gasPressure(1.*atmosphere),   // Pressure inside the gas
    temperature(273.15 *kelvin),  // temperature
    kryptonPercentage(90),        // mixture settings in molar percentage
    ch4Percentage(10),
    GasBoxLengthX(32*mm), // Length of the gas box in the X direction
    GasBoxLengthY(8*mm),  // Length of the gas box in the Y direction
    GasBoxLengthZ(130*mm) // Length of the gas box in the Z direction
{
  detectorMessenger = new DetectorMessenger(this);
}


DetectorConstruction::~DetectorConstruction() {
  delete detectorMessenger;
}

G4VPhysicalVolume* DetectorConstruction::Construct(){

    //Colors for visualization
    G4VisAttributes* red = new G4VisAttributes(G4Colour(1., 0., 0.));
    G4VisAttributes* green = new G4VisAttributes(G4Colour(0., 1., 0.));
    G4VisAttributes* blue = new G4VisAttributes(G4Colour(0., 0., 1.));
    G4VisAttributes* yellow = new G4VisAttributes(G4Colour(1.0, 1.0, 0.));
    G4VisAttributes* purple = new G4VisAttributes(G4Colour(1.0, 0., 1.0));

    /*
    #################################
    ########### WORLD VOLUME ########
    #################################
    */

    G4NistManager* man = G4NistManager::Instance();
    man->SetVerbose(1);

    G4Material* worldMat = man->FindOrBuildMaterial("G4_Galactic");
    G4VSolid* worldSolid = new G4Box("worldSolid", worldHalfLength, worldHalfLength, worldHalfLength);
    G4LogicalVolume* worldLogical = new G4LogicalVolume(worldSolid, worldMat, "WorldLogical");
    G4VPhysicalVolume* worldPhysical = new G4PVPlacement(
      nullptr,                        // no rotation
      G4ThreeVector(0, 0, 0),         // placement position
      worldLogical,                   // logical volume to place
      "WorldPhysical",                // name
      nullptr,                        // mother volume
      false,                          // no boolean operations
      0,                              // copy number
      checkOverlaps                   // check for overlaps
    );

    /*
    #################################
    ########### MATERIALS ###########
    #################################
    */

    /*First: build materials
        First cylinder: He
        Second cylinder: Kr + CH4 at a certain flux
        Third cylinder: Kr + CH4 at a certain flux
        Gas: mixture of Kr and CH4
        Anodes: Silicon 
    */

    // Defining the gas elements: He, Kr and CH4
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

    /*
    #################################
    ########### DETECTOR GEOMETRY####
    #################################
    */

    G4GDMLParser parser; 
    parser.Read("../../DMPX/World.gdml");
    G4LogicalVolume* cadObjectsLogical = parser.GetVolume("__vol__11_");
    
    if (!cadObjectsLogical) {
        G4cerr << "Error: Logical volume 'Detector_1' not found!" << G4endl;
        return nullptr;
    }

    // Placing the CAD objects inside our existing world
    new G4PVPlacement(
      nullptr,                         // no rotation
      G4ThreeVector(0,0,0),            // placement position
      cadObjectsLogical,               // logical volume to place
      "cadObjectsPhys",                // name
      worldLogical,                    // mother volume
      false,                           // no boolean operations
      0,                               // copy number
      true                             // check for overlaps
    );
    

    /*
    #################################
    ########### GAS VOLUME ##########
    #################################
    */

    G4double gasbox_x = 32*mm; // Dimensions in absolute length of the gas box in X, Y and Z (from Camenen's thesis)
    G4double gasbox_y = 8*mm;
    G4double gasbox_z = 130*mm;

    G4Box* KrCH4GasBox = new G4Box("GasBox", gasbox_x/2, gasbox_y/2, gasbox_z/2);
    logicGasBox = new G4LogicalVolume(KrCH4GasBox, KrCH4_90_10, "GasBoxLogical");

    G4VisAttributes* gasVis = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0, 0.3)); // RGBA: Blue with 30% opacity
    gasVis->SetForceSolid(true);  // Makes sure the volume is drawn as a surface
    logicGasBox->SetVisAttributes(gasVis);
    G4Region* gasRegion = new G4Region("GasRegion");
    gasRegion->AddRootLogicalVolume(logicGasBox);

    // Placing our gas volume inside the world
    new G4PVPlacement(
        0,                            // no rotation
        G4ThreeVector(-21*mm, 0.9*mm, (65+17)*mm),  // Placement position
        logicGasBox,                    // logical volume to place
        "physGasBox",                 // name
        worldLogical,                 // mother volume       
        false,                        // no boolean operations
        0,                            // copy number
        true                          // check for overlaps
    );

    return worldPhysical; 
}

void DetectorConstruction::ConstructSDandField(){

  /*
  #################################
  ########### ELECTRIC FIELD ######
  #################################
  */

  // Define a constant electric field
  // G4ThreeVector fieldVector(0.0, -100.0 * kilovolt / cm, 0.0); // Example: 1 kV/cm in the Z direction
  // pEMfield = new G4UniformElectricField(fieldVector);

  // Create an equation of motion for the field
  // pEquation = new G4EqMagElectricField(pEMfield);

  // Create a Runge-Kutta stepper
  // G4int nvar = 8; // Number of variables for integration
  // auto pStepper = new G4DormandPrince745(pEquation, nvar);

  // Create an integration driver
  // G4double minStep = 0.01 * mm; // Minimum step size
  // auto pIntegrationDriver = new G4IntegrationDriver<G4DormandPrince745>(minStep, pStepper, nvar);

  // Create a chord finder
  // pChordFinder = new G4ChordFinder(pIntegrationDriver);

  // Get the global field manager
  // auto fieldManager = G4TransportationManager::GetTransportationManager()->GetFieldManager();

  // Set the field and chord finder in the field manager
  // fieldManager->SetDetectorField(pEMfield);
  // fieldManager->SetChordFinder(pChordFinder);

  // Attach the field manager to the world logical volume
  // G4LogicalVolume* worldLogical = G4LogicalVolumeStore::GetInstance()->GetVolume("WorldLogical");
  // if (worldLogical) {
  //     worldLogical->SetFieldManager(fieldManager, true);
  // } else {
  //     G4cerr << "Error: World logical volume not found!" << G4endl;
  // }

  G4LogicalVolume* logicGasBox = G4LogicalVolumeStore::GetInstance()->GetVolume("GasBoxLogical");
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

  // Check the logical volume store
  // auto store = G4LogicalVolumeStore::GetInstance();
  // G4cout << "=== Logical Volumes in Store ===" << G4endl;
  // for (auto vol : *store) {
  //     G4cout << " - " << vol->GetName() << G4endl;
  // }

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

