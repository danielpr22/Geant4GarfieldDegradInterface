// Included from the current project
#include "../include/DetectorConstruction.hh"
#include "../include/DetectorMessenger.hh"
#include "../include/GasBoxSD.hh"
#include "../include/HeedDeltaElectronModelGas.hh"
#include "../include/HeedDeltaElectronModelAnodes.hh"
#include "../include/DegradModel.hh"
#include "../include/AnodesSD.hh"
#include "../include/GasModelParameters.hh"

// Included from the loaded libraries (G4, ROOT, Garfield++, Degrad...)
#include "G4GDMLParser.hh"
#include "G4Tubs.hh" // For the anodes
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

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::DetectorConstruction(GasModelParameters* gmp):
    fGasModelParameters(gmp),
    checkOverlaps(0),
    worldHalfLength(0.2*m),        // World volume is a cube with side length = 3m;
    gasPressure(1.*atmosphere),   // Pressure inside the gas
    temperature(273.15 *kelvin),  // temperature
    kryptonPercentage(90),        // mixture settings in molar percentage
    ch4Percentage(10),
    GasBoxLengthX(130*mm), // Length of the gas box in the X direction
    GasBoxLengthY(32*mm),  // Length of the gas box in the Y direction
    GasBoxLengthZ(8*mm), // Length of the gas box in the Z direction
    GasBoxCenterPositionX(0.*mm), // X position of the gas box center
    GasBoxCenterPositionY(0.*mm), // Y position of the gas box center
    GasBoxCenterPositionZ(0.*mm) // Z position of the gas box center
    // GasBoxCenterPositionX(-21*mm), // X position of the gas box center
    // GasBoxCenterPositionY(0.9*mm), // Y position of the gas box center
    // GasBoxCenterPositionZ(30*mm) // Z position of the gas box center
    //GasBoxCenterPositionZ(82*mm) // Z position of the gas box center
{
  // "This" is a pointer that is conceptually equivalent to the "self" in Python
  detectorMessenger = new DetectorMessenger(this);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction() {
  delete detectorMessenger;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct() {

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
    man->SetVerbose(0);

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
        Anodes: Ask Oulfa for the exact material
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
    G4double molarMassKr = 83.798;  // g/mol
    G4double molarMassCH4 = 16.04;  // g/mol
    G4double densityKr = 0.00375;   // g/cm3
    G4double densityCH4 = 0.000716; // g/cm3
    G4double massFractionKr = (kryptonPercentage * molarMassKr) / ((kryptonPercentage * molarMassKr) + (ch4Percentage * molarMassCH4));
    G4double massFractionCH4 = 1 - massFractionKr;

    // Defining the gas density and mixture by fractional mass
    G4double density = (massFractionKr * densityKr +  massFractionCH4 * densityCH4); // g/cm3
    G4Material* KrCH4_90_10 = new G4Material("KrCH4_90_10", density, 2, kStateGas, temperature, gasPressure);
    KrCH4_90_10->AddMaterial(Krypton, massFractionKr);  // 97.3% by mass (90% molar)
    KrCH4_90_10->AddMaterial(Methane, massFractionCH4); // 2.7% by mass (10% molar)

    /*
    #################################
    ########### ANODES ##############
    #################################
    */

    G4Material* anodesMat = man->FindOrBuildMaterial("G4_Au"); // e_ionisation = 790 eV | density = 19.32 g/cm3
    G4double anodesLengthX = 32*mm;
    G4double anodesR = 0.1*mm; // 20 micrometers in diameter (thus 10 in radius)
    G4double anodesSpacing = 2*mm; // 2 mm spacing between the anodes
    G4int nbOfAnodes = 64; 

    G4VSolid* anodeSolid = new G4Tubs("AnodeSolid", 0, anodesR, anodesLengthX, 0, twopi);
    G4LogicalVolume* anodeLogical = new G4LogicalVolume(anodeSolid, anodesMat, "AnodeLogical");

    G4Region* anodesRegion = new G4Region("AnodesRegion");
    anodesRegion->AddRootLogicalVolume(anodeLogical); // Adding the anodes to ROOT to process them with Garfield++

    G4RotationMatrix* rotAnodes = new G4RotationMatrix();
    rotAnodes->rotateX(90*deg); // Rotate the anodes to be perpendicular to the gas box

    for (G4int i = 0; i < nbOfAnodes; i++) {
        G4double xPos = anodesSpacing * (i - nbOfAnodes/2); 
        G4double yPos = 0;
        G4double zPos = 5*mm;
        new G4PVPlacement(
            rotAnodes,                        // rotation perpendicular to the gas box
            G4ThreeVector(xPos, yPos, zPos), // placement position
            anodeLogical,                   // logical volume to place
            "AnodePhysical",                // name
            worldLogical,                        // mother volume
            false,                          // no boolean operations
            i,                              // copy number
            checkOverlaps                   // check for overlaps
        );
    }
  

    /*
    #################################
    ########### GAS VOLUME ##########
    #################################
    */

    G4Box* KrCH4GasBox = new G4Box("GasBox", GasBoxLengthX/2, GasBoxLengthY/2, GasBoxLengthZ/2);
    logicGasBox = new G4LogicalVolume(KrCH4GasBox, KrCH4_90_10, "GasBoxLogical");

    G4Region* gasRegion = new G4Region("GasRegion");
    gasRegion->AddRootLogicalVolume(logicGasBox);

    G4cout << "(Debug: DetectorConstruction.cc) The gas box is made of " << logicGasBox->GetMaterial()->GetName() << G4endl;

    // Placing our gas volume inside the world
    new G4PVPlacement(
        0,                            // no rotation
        G4ThreeVector(GasBoxCenterPositionX, GasBoxCenterPositionY, GasBoxCenterPositionZ),  // Placement position (centered on the slit's position)
        logicGasBox,                    // logical volume to place
        "physGasBox",                 // name
        worldLogical,                 // mother volume
        false,                        // no boolean operations
        0,                            // copy number
        checkOverlaps                 // check for overlaps
    );


    /*
    #################################
    ########### VISUALIZATION########
    #################################
    */

    worldLogical->SetVisAttributes(G4VisAttributes::GetInvisible());
    G4VisAttributes* gasVis = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0, 0.3)); // RGBA: Blue with 30% opacity
    gasVis->SetForceSolid(true);  // Makes sure the volume is drawn as a surface
    logicGasBox->SetVisAttributes(gasVis);
    anodeLogical->SetVisAttributes(red);


    return worldPhysical;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::ConstructSDandField(){

  G4double thermalE = fGasModelParameters->GetThermalEnergy();
  G4cout << "(Debug: DetectorConstruction.cc) The thermal energy has been set to: " << thermalE / eV << " eV" << G4endl;

  // Getting the logical volumes from the store
  G4LogicalVolume* logicGasBox = G4LogicalVolumeStore::GetInstance()->GetVolume("GasBoxLogical");
  if (!logicGasBox) {
      G4cerr << "(Error: DetectorConstruction.cc) Logical volume 'GasBoxLogical' not found!" << G4endl;
      return;
  }

  G4LogicalVolume* logicAnodes = G4LogicalVolumeStore::GetInstance()->GetVolume("AnodeLogical");
  if (!logicAnodes) {
    G4cerr << "(Error: DetectorConstruction.cc) Logical volume 'AnodeLogical' not found!" << G4endl;
    return;
  }

  // Initializing the sensitive detector manager
  G4SDManager* SDManager = G4SDManager::GetSDMpointer();
  SDManager->SetVerboseLevel(0); // Set silent

  // Defining the gas box as a sensitive detector for Degrad and then Garfield++, once the avalanche has been calculated
  G4String KrCH4GasBoxSDname = "interface/KrCH4GasBoxSD";
  GasBoxSD* KrCH4GasBoxSD = new GasBoxSD(KrCH4GasBoxSDname); // GasBoxSD of type G4SensitiveDetector
  SDManager->AddNewDetector(KrCH4GasBoxSD);
  SetSensitiveDetector(logicGasBox,KrCH4GasBoxSD);

  // Defining the anodes as sensitive detectors for the HeedDeltaElectronModel
  G4String AnodesSDname = "interface/AnodesSD";
  AnodeSD* AnodesSD = new AnodeSD(AnodesSDname); // AnodesSD of type G4SensitiveDetector
  SDManager->AddNewDetector(AnodesSD);
  SetSensitiveDetector(logicAnodes,AnodesSD);

  // Check the logical volume store for debugging
  auto store = G4LogicalVolumeStore::GetInstance();
  G4cout << "(Debug: DetectorConstruction.cc) === Logical Volumes in Store ===" << G4endl;
  for (auto vol : *store) {
      G4cout << " - " << vol->GetName() << G4endl;
  }

  // These commands generate the two gas models (Degrad and HeedeltaElectron) and connect them 
  // to the GasRegion and the AnodesRegion
  G4Region* GasRegion = G4RegionStore::GetInstance()->GetRegion("GasRegion");
  G4Region* AnodesRegion = G4RegionStore::GetInstance()->GetRegion("AnodesRegion");
  
  new DegradModel(fGasModelParameters,"DegradModel",GasRegion,this,KrCH4GasBoxSD);
  G4cout << "(Debug: DetectorConstruction.cc) Gas region connected with DegradModel..." << G4endl;


  // Attaching the HeedDeltaElectronModel to the anodes, for the signal calculation
  new HeedDeltaElectronModelAnodes(fGasModelParameters,"HeedDeltaElectronModelAnodes",AnodesRegion,this,AnodesSD);
  G4cout << "(Debug: DetectorConstruction.cc) Anode region connected with HeedDeltaElectronModel..." << G4endl;
  
  // Attaching the HeedDeltaElectronModel to the gas region, for the drift
  new HeedDeltaElectronModelGas(fGasModelParameters,"HeedDeltaElectronModelGas",GasRegion,this,KrCH4GasBoxSD);
  G4cout << "(Debug: DetectorConstruction.cc) Gas region connected with HeedDeltaElectronModel..." << G4endl;



}

