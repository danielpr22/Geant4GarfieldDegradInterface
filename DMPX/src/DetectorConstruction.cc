// Included from the current project
#include "../include/DetectorConstruction.hh"
#include "../include/DetectorMessenger.hh"
#include "../include/GasBoxSD.hh"
#include "../include/HeedDeltaElectronModel.hh"
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
    fGasModelParameters(gmp) {

  detectorMessenger = new DetectorMessenger(this);

  // World, overlaps, pressure and temperature settings
  worldHalfLength = detectorMessenger->GetWorldHalfLength() / cm;
  checkOverlaps = detectorMessenger->GetCheckOverlaps();
  gasPressure = detectorMessenger->GetPressure();

  // Gas percentages
  kryptonPercentage = detectorMessenger->GetKryptonPercentage();
  ch4Percentage = detectorMessenger->GetCH4Percentage();
  
  // Settings for the gas box
  // Unifying the units for the positioning
  GasBoxLengthX = detectorMessenger->GetGasBoxLengthX() / cm;
  GasBoxLengthY = detectorMessenger->GetGasBoxLengthY() / cm;
  GasBoxLengthZ = detectorMessenger->GetGasBoxLengthZ() / cm;
  GasBoxCenterPositionX = detectorMessenger->GetGasBoxCenterPositionX() / cm;
  GasBoxCenterPositionY = detectorMessenger->GetGasBoxCenterPositionY() / cm;
  GasBoxCenterPositionZ = detectorMessenger->GetGasBoxCenterPositionZ() / cm;
  
  // Anode settings
  anodesHalfLength = detectorMessenger->GetAnodesHalfLength() / cm;
  anodesR = detectorMessenger->GetAnodesR() / cm;
  anodesSpacing = detectorMessenger->GetAnodesSpacing() / cm;
  nbOfAnodes = detectorMessenger->GetNbOfAnodes() / cm;

  // Cathode settings
  cathodes1_LengthX = Getcathodes1_LengthX() / cm; 
  cathodes1_LengthY = Getcathodes1_LengthY() /cm ; 
  cathodes1_LengthZ = Getcathodes1_LengthZ() / cm; 
  cathodes1_XPos = Getcathodes1_XPos() / cm; 
  cathodes1_ZPos = Getcathodes1_ZPos() /cm; 
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction() {
  delete detectorMessenger;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct() {

    //Colors for visualization
    G4VisAttributes* red = new G4VisAttributes(G4Colour(1., 0., 0., 0.3)); // Color and opacity
    G4VisAttributes* green = new G4VisAttributes(G4Colour(0., 1., 0., 0.3));
    G4VisAttributes* blue = new G4VisAttributes(G4Colour(0., 0., 1., 0.3));
    G4VisAttributes* yellow = new G4VisAttributes(G4Colour(1.0, 1.0, 0., 0.3));
    G4VisAttributes* purple = new G4VisAttributes(G4Colour(1.0, 0., 1.0, 0.3));
    
    /*
    #################################
    ########### WORLD VOLUME ########
    #################################
    */

    G4NistManager* man = G4NistManager::Instance();

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
        Gas boxes: mixtures of Kr and CH4 90/10
        Cathodes: Be
        Anodes: Gold-coated tungsten (the material is not important as it is treated by Garfield++ as a conductor)
    */

    // Defining the gas elements: He, Kr and CH4
    G4Material* Helium = man->FindOrBuildMaterial("G4_He");
    G4Material* Krypton = man->FindOrBuildMaterial("G4_Kr");
    G4Material* Methane = man->FindOrBuildMaterial("G4_METHANE");
    G4Material* anodesMat = man->FindOrBuildMaterial("G4_Au"); // e_ionisation = 790 eV | density = 19.32 g/cm3
    G4Material* Beryllium = man->FindOrBuildMaterial("G4_Be"); 

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
    G4double molarMassKr = 83.798 * g/mole;  // g/mol
    G4double molarMassCH4 = 16.04 * g/mole;  // g/mol
    G4double densityKr = 0.00375 * g/cm3;   // g/cm3
    G4double densityCH4 = 0.000716 * g/cm3; // g/cm3
    G4double massFractionKr = (kryptonPercentage/100 * molarMassKr) / ((kryptonPercentage/100 * molarMassKr) + (ch4Percentage/100 * molarMassCH4));
    G4double massFractionCH4 = 1 - massFractionKr;
    temperature = fGasModelParameters->GetTemperature(); 

    // Defining the gas density and mixture by fractional mass
    G4double density = (massFractionKr * densityKr +  massFractionCH4 * densityCH4); // g/cm3

    G4cout << "(Debug: DetectorConstruction.cc) The density of the gas is: " 
          << G4BestUnit(density, "Volumic Mass") << G4endl;

    G4cout << "(Debug: DetectorConstruction.cc) The temperature is set to: " << G4BestUnit(temperature, "Temperature") << G4endl; 
    
    G4Material* KrCH4_90_10 = new G4Material("KrCH4_90_10", density, 2, kStateGas, temperature, gasPressure);
    KrCH4_90_10->AddMaterial(Krypton, massFractionKr);  // 97.3% by mass (90% molar)
    KrCH4_90_10->AddMaterial(Methane, massFractionCH4); // 2.7% by mass (10% molar)

    /*
    #################################
    ########### GAS VOLUME ##########
    #################################
    */

    G4Box* KrCH4GasBox = new G4Box("GasBox", GasBoxLengthX/2, GasBoxLengthY/2, GasBoxLengthZ/2); // Geant4 reads half lengths
    logicGasBox = new G4LogicalVolume(KrCH4GasBox, KrCH4_90_10, "GasBoxLogical");

    G4Region* gasAndAnodesRegion = new G4Region("GasAndAnodesRegion");
    logicGasBox->SetRegion(gasAndAnodesRegion); // We set the region for the gas box
    gasAndAnodesRegion->AddRootLogicalVolume(logicGasBox); // We add the gas box to the root logical volume 

    G4cout << "(Debug: DetectorConstruction.cc) The gas box is made of: " << logicGasBox->GetMaterial()->GetName() << G4endl;

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
    ########### ANODES ##############
    #################################
    */

    G4VSolid* anodeSolid = new G4Tubs("AnodeSolid", 0, anodesR, anodesHalfLength, 0, twopi);
    G4LogicalVolume* anodeLogical = new G4LogicalVolume(anodeSolid, anodesMat, "AnodeLogical");

    for (G4int i = 0; i < nbOfAnodes; i++) {
        G4double xPos = anodesSpacing * (i - nbOfAnodes/2);
        G4cout << "(Debug: DetectorConstruction.cc) The position of the anode is: " << xPos << G4endl;
        G4double yPos = GasBoxCenterPositionY;
        G4double zPos = GasBoxCenterPositionZ;

        new G4PVPlacement(
            0,                                // no rotation
            G4ThreeVector(xPos, yPos, zPos),  // placement position
            anodeLogical,                     // logical volume to place
            "AnodePhysical",                  // name
            logicGasBox,                      // The mother volume is the gas region
            false,                            // no boolean operations
            i,                                // copy number
            checkOverlaps                     // check for overlaps
        );
        anodeLogical->SetRegion(gasAndAnodesRegion); 
        gasAndAnodesRegion->AddRootLogicalVolume(anodeLogical); // We add the anodes to the root logical volume
    }


    /*
    #################################
    ########### CATHODES ############
    #################################
    */

    // The cathodes are treated by Geant4, therefore no need to add the ROOT logical volume

    G4VSolid* cathode1Solid= new G4Box("CathodeSolid", cathodes1_LengthX/2, cathodes1_LengthY/2, cathodes1_LengthZ/2); // Geant4 reads half lenghts
    G4LogicalVolume* cathode1Logical = new G4LogicalVolume(cathode1Solid, Beryllium, "Cathode1Logical"); 
    
    // Up cathode for the first MPX
    new G4PVPlacement(
      0,                            // no rotation
      G4ThreeVector(cathodes1_XPos, cathodes1_LengthY/2 + GasBoxLengthY/2, cathodes1_ZPos),
      cathode1Logical,                    // logical volume to place
      "physCathode1Up",                 // name
      worldLogical,                 // mother volume
      false,                        // no boolean operations
      0,                            // copy number
      checkOverlaps                 // check for overlaps
    );

    // Down cathode for the second MPX
    new G4PVPlacement(
      0,                            // no rotation
      G4ThreeVector(cathodes1_XPos, -cathodes1_LengthY/2 - GasBoxLengthY/2, cathodes1_ZPos),
      cathode1Logical,                    // logical volume to place
      "physCathode1Down",                 // name
      worldLogical,                 // mother volume
      false,                        // no boolean operations
      0,                            // copy number
      checkOverlaps                 // check for overlaps
    );

    /*
    #################################
    ########### VISUALIZATION #######
    #################################
    */

    worldLogical->SetVisAttributes(G4VisAttributes::GetInvisible());
    logicGasBox->SetVisAttributes(blue);
    anodeLogical->SetVisAttributes(red);
    cathode1Logical->SetVisAttributes(green); 

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

  // Initializing the sensitive detector manager
  G4SDManager* SDManager = G4SDManager::GetSDMpointer();

  // Defining the gas box as a sensitive detector for Degrad and then Garfield++, once the avalanche has been calculated
  G4String KrCH4GasBoxSDname = "interface/KrCH4GasBoxSD";
  fGasBoxSD = new GasBoxSD(KrCH4GasBoxSDname); // For the gas box getter method
  SDManager->AddNewDetector(fGasBoxSD);
  SetSensitiveDetector(logicGasBox,fGasBoxSD);

  // Check the logical volume store for debugging
  auto store = G4LogicalVolumeStore::GetInstance();
  G4cout << "(Debug: DetectorConstruction.cc) === Logical Volumes in Store ===" << G4endl;
  for (auto vol : *store) {
      G4cout << " - " << vol->GetName() << G4endl;
  }

  // Check the region store for debugging
  auto reg_store = G4RegionStore::GetInstance();
  G4cout << "(Debug: DetectorConstruction.cc) === Regions in Store ===" << G4endl;
  for (auto reg : *reg_store) {
    G4cout << " - " << reg->GetName() << G4endl;
  }

  G4Region* gasAndAnodesRegion = G4RegionStore::GetInstance()->GetRegion("GasAndAnodesRegion");
  
  // These commands generate the two gas models (Degrad and HeedeltaElectron) and connect them 
  // to the region formed by the gas and the anodes
  fDegradModel = new DegradModel(fGasModelParameters,"DegradModel",gasAndAnodesRegion,this,fGasBoxSD);
  G4cout << "(Debug: DetectorConstruction.cc) Gas & anodes region connected with DegradModel..." << G4endl;

  // // Attaching the HeedDeltaElectronModel to the anodes, for the signal calculation
  fHeedDeltaElectronModel = new HeedDeltaElectronModel(fGasModelParameters,"HeedDeltaElectronModel",gasAndAnodesRegion,this,fGasBoxSD);
  G4cout << "(Debug: DetectorConstruction.cc) Gas & anodes region connected with HeedDeltaElectronModel..." << G4endl;
}

