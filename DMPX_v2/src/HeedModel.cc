#include <iostream>
#include <stdio.h>

#include "../include/HeedModel.hh"
#include "../include/DriftLineTrajectory.hh"
#include "../include/DetectorConstruction.hh"
#include "../include/EventAction.hh"

#include "G4VPhysicalVolume.hh"
#include "G4Electron.hh"
#include "G4Gamma.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4RunManager.hh"
#include "G4TrackingManager.hh"
#include "G4EventManager.hh"
#include "G4VVisManager.hh"
#include "SolidBox.hh"
#include "SolidTube.hh"
#include "Garfield/ViewGeometry.hh" // Include the correct header for Garfield::ViewGeometry
#include "G4AutoLock.hh"
#include "MediumMagboltz.hh"

namespace{G4Mutex aMutex = G4MUTEX_INITIALIZER;}

const static G4double torr = 1. / 760. * atmosphere;

HeedModel::HeedModel(GasModelParameters* gmp, G4String modelName, G4Region* envelope, DetectorConstruction* dc, GasBoxSD* sd)
: G4VFastSimulationModel(modelName, envelope), detCon(dc), fGasBoxSD(sd), fGasModelParameters(gmp){
  
  /////////////////////////////////////////////////////////////////////////////////////
  // IMPORTANT: In Garfield++, the distances are expressed in cm and the times in ns //
  /////////////////////////////////////////////////////////////////////////////////////

  thermalE = gmp->GetThermalEnergy(); // eV
  temperature = gmp->GetTemperature(); // Kelvin
  fVisualizeChamber = gmp->GetVisualizeChamber();
  fVisualizeSignal = gmp->GetVisualizeSignals();
  fVisualizeField = gmp->GetVisualizeField();
  numberOfEvents = 0;
  secondaryElectronCounterTotal = 0;
  jumpDriftStepPoints = gmp->GetJumpDriftStepPoints();
  gasAmplificationCoefficient = 0.0; 

  // We get the construction parameters from the DetectorConstruction class
  nameOfSimulation = GetNameOfSimulation(*detCon); // Name used for the storing of results
  anodesHalfLength = GetAnodesHalfLength(*detCon) / CLHEP::cm;
  anodesR = GetAnodesR(*detCon) / CLHEP::cm;
  anodesSpacing = GetAnodesSpacing(*detCon) / CLHEP::cm;
  nbOfAnodes = GetNbOfAnodes(*detCon);
  cathodes_1_LengthX = GetCathodes1_LengthX(*detCon); 
  cathodes_1_LengthY = GetCathodes1_LengthY(*detCon); 
  cathodes_1_LengthZ = GetCathodes1_LengthZ(*detCon); 
  cathodes_1_XPos = GetCathodes1_XPos(*detCon); 
  cathodes_1_ZPos = GetCathodes1_ZPos(*detCon); 

  // Temporary variable to store the old values from the anode and cathode voltages, and check if they have changed
  vAnodeWires_temp = 0.0; 
  vCathodePlane_temp = 0.0; 

  // The distance between the anodes and cathodes is equal to the gas box height
  distanceAnodeCathodes = GetGasBoxLengthY(*detCon);  

  // Vector to store the number of electrons in each wire
  std::vector<int> electronsInWires; 

  // The Heed Model, as the Degrad model, is only created once at the beginning, it is not recreated after each event
  G4cout << "(Debug: HeedModel.cc) Heed Model has been successfully created..." << G4endl;
}

HeedModel::~HeedModel() {}

//Method called when a particle is created, checks if the model is applicable for this particle
G4bool HeedModel::IsApplicable(const G4ParticleDefinition& particleType) {
  if (particleType.GetParticleName()=="e-") {
		G4cout << "(Debug: HeedModel.cc) Electron generated, the model is applicable..." << G4endl;
		return true;
	}
	return false;	
}

//Method called in every step: checks if the conditions of the particle are met. If true the DoIt-method is called
G4bool HeedModel::ModelTrigger(const G4FastTrack& fastTrack) {
  G4cout << "(Debug: HeedModel.cc) Inside the ModelTrigger method..." << G4endl;
  G4double ekin = fastTrack.GetPrimaryTrack()->GetKineticEnergy();
  G4cout << "(Debug: HeedModel.cc) The kinetic energy of the particle is: " << G4BestUnit(ekin, "Energy") << G4endl;
  if (ekin<=thermalE) {
		G4cout << "(Debug: HeedModel.cc) Triggered! The Garfield model is triggered below energies of " <<  G4BestUnit(thermalE, "Energy") << G4endl;
		return true;
  }
  else {return false;} 
}

//Implementation of the general model, the Run method, called at the end, is specifically implemented for the daughter classes
void HeedModel::DoIt(const G4FastTrack& fastTrack, G4FastStep& fastStep) {

  // Getting the EventAction object for the number of events
  auto eventAction = dynamic_cast<EventAction*>(
    const_cast<G4UserEventAction*>(G4RunManager::GetRunManager()->GetUserEventAction()));

  if(eventAction) {
    if (shotNumber != eventAction->GetShotNumber()) { // If the shot number is different, then we reset the number of events
      shotNumber = eventAction->GetShotNumber();
      numberOfEvents = 0;
      secondaryElectronCounterTotal = 0; // And the total number of secondary electrons
      electronsInWires.clear(); // Clear the vector to store the number of electrons in each wire
      electronsInWires.resize(nbOfAnodes, 0); // Reinitialize the vector to store the number of electrons in each wire
      G4cout << "(Debug: HeedModel.cc) The shot number has changed, resetting the number of events..." << G4endl;
    }
  }
    
  G4cout << "(Debug: HeedModel.cc) The shot number is: " << shotNumber << G4endl;

  fastStep.KillPrimaryTrack(); // We first kill the Degrad tracks
  G4cout << "(Debug: HeedModel.cc) The electron has been killed..." << G4endl; 

  G4ThreeVector dir = fastTrack.GetPrimaryTrack()->GetMomentumDirection();
  G4ThreeVector worldPosition = fastTrack.GetPrimaryTrack()->GetPosition();
  G4cout << "(Debug: HeedModel.cc) The position of the particle before drift is: " << G4BestUnit(worldPosition, "Length") << G4endl; 

  G4double ekin = fastTrack.GetPrimaryTrack()->GetKineticEnergy();
  G4cout << "(Debug: HeedModel.cc) The kinetic energy of the particle is: " << G4BestUnit(ekin, "Energy") << G4endl;

  G4double time = fastTrack.GetPrimaryTrack()->GetGlobalTime();
  G4String particleName =
      fastTrack.GetPrimaryTrack()->GetParticleDefinition()->GetParticleName();

  // This Run method calls HeedDeltaElectronModel
  Run(fastStep, fastTrack, particleName, ekin, time, worldPosition.x() / CLHEP::cm,
      worldPosition.y() / CLHEP::cm, worldPosition.z() / CLHEP::cm,
      dir.x(), dir.y(), dir.z());
}

//Checks if the particle is in the list of particle for which the model is applicable (called by IsApplicable)
G4bool HeedModel::FindParticleName(G4String name) {
  MapParticlesEnergy::iterator it;
  it = fMapParticlesEnergy.find(name);
  if (it != fMapParticlesEnergy.end()) {
    return true;
  }
  return false;
}

//Checks if the energy condition of the particle is in the list of conditions for which the model shoould be triggered (called by ModelTrigger)
G4bool HeedModel::FindParticleNameEnergy(G4String name, double ekin_keV) {
  MapParticlesEnergy::iterator it;
  for (it=fMapParticlesEnergy.begin(); it!=fMapParticlesEnergy.end();++it) {
    if(it->first == name) {
      EnergyRange_keV range = it->second;
      G4cout << "(Debug: HeedModel.cc) The energy range is: " << G4BestUnit(range.first, "Energy") << " - " << G4BestUnit(range.second, "Energy") << G4endl;
      if (range.first <= ekin_keV && range.second >= ekin_keV) {
        return true;
      }
    }
  }
  return false;
}

// Initialize the Garfield++ related geometries and physics/tracking mechanisms, this is specific for each use and 
// should be re-implemented entirely. 

//////////////////////////////////////////////////////////////////////////////////
// IMPORTANT: These functions should mimick the situation represented by Geant4 //
//////////////////////////////////////////////////////////////////////////////////

void HeedModel::InitialisePhysics(){
  if(G4RunManager::GetRunManager()->GetRunManagerType() == G4RunManager::workerRM || G4RunManager::GetRunManager()->GetRunManagerType() == G4RunManager::sequentialRM){
    makeGas();
    buildBoxAndField(); 
    BuildSensor();
    SetTracking();

    // Applying the configuration for the plots (but the plots are made in PlotTrack, once the
    // data has been retrieved).
    if(fVisualizeChamber) SettingChamberView();
    if(fVisualizeSignal) SettingSignalView();
    if(fVisualizeField) SettingFieldView();

    G4cout << "(Debug: HeedModel.cc) The HeedModel has been initialized..." << G4endl; 
  }
}

// Gas intialization (see Garfield++ documentation)
void HeedModel::makeGas(){
  fMediumMagboltz = new Garfield::MediumMagboltz();
  double pressure = detCon->GetGasPressure()/torr;
  double krPerc = detCon->GetKryptonPercentage();
  double ch4Perc = detCon->GetCH4Percentage();
  fMediumMagboltz->SetComposition("kr", krPerc, "ch4", ch4Perc);
  fMediumMagboltz->SetPressure(pressure); 
  fMediumMagboltz->SetTemperature(temperature);

  G4cout << "(Debug: HeedModel.cc) Now writing the gas file..." << G4endl;

  // ####################
  // ##### MAGBOLTZ #####
  // ####################

  // // Please, comment out the following lines if you do not want to use Magboltz to 
  // // generate the gas file. The gas file is generated only once for a given gas configuration,
  // // and then it is used for the rest of the simulations.

  // // Generating the gas file for the current gas mixture
  // // SetFieldGrid(Min electric field [V/cm], max electric field [V/cm], number of points in the field grid, 
  // // and whether to use log spacing)
  // fMediumMagboltz->SetFieldGrid(100.0, 100.e3, 20., true); 

  // // Number of collisions (in multiples of 10^7) over which the electron is traced by Magboltz
  // const int ncoll = 10; 

  // // Generating the gas table (.gas file)
  // fMediumMagboltz->GenerateGasTable(ncoll);

  // /// We save the generated file for later use
  // fMediumMagboltz->WriteGasFile("kr_90_ch4_10.gas");


  fMediumMagboltz->EnableDebugging();
  fMediumMagboltz->Initialise(true);
  fMediumMagboltz->DisableDebugging();

  G4cout << "(Debug: HeedModel.cc) The gas file is: " << gasFile << G4endl;
  const std::string path = getenv("GARFIELD_HOME");
  G4AutoLock lock(&aMutex);

  // The ion mobility file allows for a more precise modelling of the ion tail in the avalanche
  if(ionMobFile!="")
    fMediumMagboltz->LoadIonMobility(path + "/Data/" + ionMobFile);
    G4cout << "(Debug: HeedModel.cc) The ion mobility file is searched in the Garfield database: " << ionMobFile << G4endl;
  
  // The gas file, generated by Magboltz, computes the necessary modelization parameters for our gas mixture
  if(gasFile!="")
  G4cout << "(Debug: HeedModel.cc) The gas file is searched in the Garfield database: " << gasFile << G4endl;
  fMediumMagboltz->LoadGasFile(gasFile.c_str());
}
  

//Geometry (see Garfield++ documentation)
void HeedModel::buildBoxAndField(){

  geo = new Garfield::GeometrySimple();

  // We must be careful with the coordinates in Garfield++ and Geant4 : the z coordinate in 
  // Garfield++ corresponds to the y coordinate in Geant4!
  box = new Garfield::SolidBox(detCon->GetGasBoxCenterPositionX()/CLHEP::cm, 
                               detCon->GetGasBoxCenterPositionY()/CLHEP::cm, 
                               detCon->GetGasBoxCenterPositionZ()/CLHEP::cm,
                               detCon->GetGasBoxLengthX()*0.5/CLHEP::cm,
                               detCon->GetGasBoxLengthY()*0.5/CLHEP::cm,
                               detCon->GetGasBoxLengthZ()*0.5/CLHEP::cm);

  G4cout << "(Debug: HeedModel.cc) The anodes half length in Y is : " << detCon->GetGasBoxLengthY()*0.5/CLHEP::cm << G4endl; 

  geo->AddSolid(box, fMediumMagboltz);
  G4cout << "(Debug: HeedModel.cc) Added gas box to Garfield geometry." << G4endl;

  // Creating the component analytic field for the field calculation
  comp = new Garfield::ComponentAnalyticField();
  comp->SetMedium(fMediumMagboltz); // Attaching the gas to the component analytic field
  comp->SetGeometry(geo);

  for (int i = 0; i < nbOfAnodes; i++) {
      const double xPos = anodesSpacing * (i - nbOfAnodes / 2); // cm
      const double yPos = detCon->GetGasBoxCenterPositionY()/CLHEP::cm; // cm
      const double zPos = detCon->GetGasBoxCenterPositionZ()/CLHEP::cm; // cm
      wire = new Garfield::SolidTube(xPos, // cm
        yPos, 
        zPos, 
        anodesR, // radius in cm
        anodesHalfLength // in cm
      ); 

      // Generate a unique name like "a_0", "a_1", ..., "a_63" for the anodes
      std::string wireName = "a_" + std::to_string(i);

      geo->AddSolid(wire, fMediumMagboltz);
      comp->AddWire(xPos, yPos, anodesR, vAnodeWires, wireName, anodesHalfLength); // Adding the wires for the signal calculation
  }

  G4cout << "(Debug: HeedModel.cc) All the wires have been added to the Garfield geometry...." << G4endl;

  // Creating the cathode plane geometry
  double cathodePlaneHalfLengthX = 0.5 * cathodes_1_LengthX / CLHEP::cm; // cm (Garfield++ works with half lengths)
  double cathodePlaneHalfLengthY = 0.5 * cathodes_1_LengthY / CLHEP::cm; // cm
  double cathodePlaneHalfLengthZ = 0.5 * cathodes_1_LengthZ / CLHEP::cm; // cm
  double xPosPlane = cathodes_1_XPos / CLHEP::cm; // cm (Positions of the center of the plane)
  double yPosPlane = 0.5 * distanceAnodeCathodes/CLHEP::cm + cathodePlaneHalfLengthY; 
  double zPosPlane = cathodes_1_ZPos / CLHEP::cm; // cm

  // The anodes are sandwiched between two cathode planes
  cathodePlane_1 = new Garfield::SolidBox(xPosPlane, yPosPlane, zPosPlane, 
    cathodePlaneHalfLengthX, cathodePlaneHalfLengthY, cathodePlaneHalfLengthZ); // The two planes are located at equal distance from the anodes
  cathodePlane_2 = new Garfield::SolidBox(xPosPlane, -yPosPlane, zPosPlane, 
    cathodePlaneHalfLengthX, cathodePlaneHalfLengthY, cathodePlaneHalfLengthZ);

  geo->AddSolid(cathodePlane_1, fMediumMagboltz);
  geo->AddSolid(cathodePlane_2, fMediumMagboltz);
  comp->AddPlaneY(yPosPlane - cathodePlaneHalfLengthY, vCathodePlane, "p_pos_y"); // Adding the cathode plane
  comp->AddPlaneY(-yPosPlane + cathodePlaneHalfLengthY, vCathodePlane, "p_neg_y"); // Adding the cathode plane

  G4cout << "(Debug: HeedModel.cc) Added cathode plane to Garfield geometry." << G4endl;

  // Additional commands for the visualization of the geometry in Garfield++
  geoView = new Garfield::ViewGeometry(geo);
  fGeoCanvas = new TCanvas("fGeoCanvas", "Geometry view", 800, 600);
  geoView->SetCanvas(fGeoCanvas);
  geoView->Plot3d();
  fGeoCanvas->Print("Geometry.png");
}


//Build sensor (see Garfield++ documentation)
void HeedModel::BuildSensor(){
  fSensor = new Garfield::Sensor();
  fSensor->AddComponent(comp);
  for (int i = 0; i < nbOfAnodes; i++) {
    std::string wireName = "a_" + std::to_string(i); // Wires are named from 0 to nbOfAnodes-1
    fSensor->AddElectrode(comp, wireName);
  }

  // For efficiency reasons, we restrict charge transport to the gas box
  fSensor->SetArea(-detCon->GetGasBoxLengthX()*0.5/CLHEP::cm, // xmin
  -detCon->GetGasBoxLengthY()*0.5/CLHEP::cm, // ymin
  -detCon->GetGasBoxLengthZ()*0.5/CLHEP::cm, // zmin
  detCon->GetGasBoxLengthX()*0.5/CLHEP::cm, // xmax
  detCon->GetGasBoxLengthY()*0.5/CLHEP::cm, // y max
  detCon->GetGasBoxLengthZ()*0.5/CLHEP::cm); // zmax
  //fSensor->SetTimeWindow(tmin, tstep, nbins); //Lowest time [ns], tstep (signal collected during this time) [ns], tfinal [ns]
}

//Set which tracking mechanism to be used: Runge-kutta, Monte-Carlo or Microscopic (see Garfield++ documentation)
void HeedModel::SetTracking(){
  if(driftRKF) {
    fDriftRKF = new Garfield::DriftLineRKF();
    fDriftRKF->SetSensor(fSensor);
    fDriftRKF->EnableDebugging();
  }
  else if(trackMicro) {
    // Multiplication is enabled in this case by default
    fAvalanche = new Garfield::AvalancheMicroscopic();
    fAvalanche->SetSensor(fSensor);
    fAvalanche->EnableSignalCalculation();
    fAvalanche->EnableDriftLines(true);
    fAvalanche->DisableDebugging();
    //fAvalanche->EnableDebugging();
  }
  else {  
    fDrift = new Garfield::AvalancheMC();
    fDrift->SetSensor(fSensor);
    fDrift->EnableSignalCalculation(); // So we can count the number of electrons and not the readout signal
    fDrift->SetDistanceSteps(1.e-5); // Step size for the drift line set to 1 micron
    G4cout << "(Debug: HeedModel.cc) The avalanche is being created..." << G4endl;
    if(createAval) {
      fDrift->DisableAvalancheSizeLimit();
      fDrift->EnableAttachment();
      fDrift->EnableMultiplication(true);
    }
    else fDrift->DisableAttachment();
  }

  // TrackHeed simulates the creation of electron/hole pairs
  fTrackHeed = new Garfield::TrackHeed();
  fTrackHeed->SetSensor(fSensor);
  fTrackHeed->SetParticle("e-");
  fTrackHeed->EnableDeltaElectronTransport();
}


// Set some visualization variables to see objects and drift lines (see Garfield++ documentation)
// This function only configures the drift object to store the points for plotting, but the
// plots are done at the end, in PlotTrack, once all the points have been retrieved.
void HeedModel::SettingChamberView(){
  G4cout << "(Debug: HeedModel.cc) Creating chamber view..." << G4endl;
  fChamberCanvas = new TCanvas("fChamberCanvas", "Chamber view", 800, 600);
  viewDrift = new Garfield::ViewDrift(); // Allocating memory for viewDrift
  viewDrift->SetCanvas(fChamberCanvas);
  G4cout << "(Debug: HeedModel.cc) Canvas creation..." << G4endl; 

  if (driftRKF) {
    G4cout << "(Debug: HeedModel.cc) DriftRKF enabled..." << G4endl; 
    fDriftRKF->EnablePlotting(viewDrift);
  }
  else if (trackMicro) {fAvalanche->EnablePlotting(viewDrift);}
  else {fDrift->EnablePlotting(viewDrift);}
  fTrackHeed->EnablePlotting(viewDrift);
  
  viewCell = new Garfield::ViewCell(); // Allocating memory for viewCell
  viewCell->SetComponent(comp);
  viewCell->SetCanvas(fChamberCanvas);
}

// Setting the view for the signal plotting (see Garfield++ documentation)
void HeedModel::SettingSignalView(){
  fSignalCanvas = new TCanvas("fSignalCanvas", "Signal on the wire", 700, 700);
  viewSignal = new Garfield::ViewSignal();
  viewSignal->SetSensor(fSensor);
  viewSignal->SetCanvas(fSignalCanvas);
}

// Setting the view for the electric field plotting (see Garfield++ documentation)
void HeedModel::SettingFieldView(){
  fFieldCanvas = new TCanvas("fFieldCanvas", "Electric field", 700, 700);
  viewField = new Garfield::ViewField();
  viewField->SetComponent(comp);
  viewField->SetCanvas(fFieldCanvas);
}


// Drift the electrons from point of creation towards the electrodes (Garfield++ documentation)
void HeedModel::Drift(double x, double y, double z, double t) {
  // Here, we have the point at the START of the drift, and we want to store all the points
  // of the drift calculation
  secondaryElectronCounter = 0; 
  electronsInWires.resize(nbOfAnodes, 0); // Initialize the vector to store the number of electrons in each wire

  // If the voltages have changed, we reinitialize the physics
  if ((vAnodeWires != vAnodeWires_temp) || (vCathodePlane != vCathodePlane_temp)) {
    G4cout << "(Debug: HeedModel.cc) The physics model has been reinitialized..." << G4endl; 

    vAnodeWires_temp = vAnodeWires; // We update the voltages
    vCathodePlane_temp = vCathodePlane; 
    comp->Clear(); // Clearing all the electrodes and planes

    // Updating the anodes
    G4cout << "(Debug: HeedModel.cc) Value of anodesSpacing: " << anodesSpacing << G4endl; 
    for (int i = 0; i < nbOfAnodes; i++) {
      const double xPos = anodesSpacing * (i - nbOfAnodes / 2); // cm
      const double yPos = detCon->GetGasBoxCenterPositionY()/CLHEP::cm; // cm
      
      // Generate a unique name like "a_0", "a_1", ..., "a_63" for the anodes
      std::string wireName = "a_" + std::to_string(i);

      comp->AddWire(xPos, yPos, anodesR, vAnodeWires, wireName, anodesHalfLength); // Adding the wires for the signal calculation
    }

    // Updating the cathodes
    double cathodePlaneHalfLengthY = 0.5 * cathodes_1_LengthY / CLHEP::cm; // cm
    double yPosPlane = 0.5 * distanceAnodeCathodes/CLHEP::cm + cathodePlaneHalfLengthY; 

    comp->AddPlaneY(yPosPlane - cathodePlaneHalfLengthY, vCathodePlane, "p_pos_y"); // Adding the cathode plane
    comp->AddPlaneY(-yPosPlane + cathodePlaneHalfLengthY, vCathodePlane, "p_neg_y"); // Adding the cathode plane

    G4cout << "(Debug: HeedModel.cc) The new plane has been added with a voltage of: " << vCathodePlane << G4endl; 

    InitialisePhysics(); // We build the sensor again with the new comp
  }

  if (driftElectrons) {
    G4cout << "(Debug: HeedModel.cc) Now drifting an electron..." << G4endl; 
    DriftLineTrajectory* dlt = new DriftLineTrajectory();
    G4TrackingManager* fpTrackingManager = G4EventManager::GetEventManager()->GetTrackingManager();
    fpTrackingManager->SetTrajectory(dlt);

    // Starting point of the drift
    G4cout << "(Debug: HeedModel.cc) Starting point of the drift: " << x << " " 
    << y <<  " " << z << " " << t << G4endl; 

    // If RK4 is to be used
    if (driftRKF) {
        fDriftRKF->DriftElectron(x, y, z, t);
        unsigned int n = fDriftRKF->GetNumberOfDriftLinePoints();
        G4cout << "(Debug: HeedModel.cc) Number of drift line points: " << n << G4endl; 
        double xi, yi, zi, ti;
        int status; // Add a variable to store the status
        for (unsigned int i = 0; i < n; i++) {
            fDriftRKF->GetDriftLinePoint(i, xi, yi, zi, ti); // To get the full trajectory
            if (G4VVisManager::GetConcreteInstance() && i % jumpDriftStepPoints == 0) { // To get all the drift step points or only some of them
                dlt->AppendStep(G4ThreeVector(xi * CLHEP::cm, yi * CLHEP::cm, zi * CLHEP::cm), ti);
            }
        }
        fDriftRKF->GetEndPoint(xi, yi, zi, ti, status); // Ths command retrieves the endpoint and end time, and the status
        if (status > 0) { // If the status is > 0, we are inside a wire (see Garfield++ documentation)
          secondaryElectronCounter++; // The secondary electron generated by Degrad has touched the wire!  
        }
    
    // If we want to use Monte-Carlo methods for the simulation of the avalanches created
    // by the secondary electrons. It is this function that allows for example to recover
    // the gas amplification factor G (average number of electrons created per secondary
    // electron generated by Degrad)
    } else if (trackMicro) {
        G4cout << "(Debug: HeedModel.cc) Now drifting an avalanche..." << G4endl;
        fAvalanche->AvalancheElectron(x, y, z, t, thermalE, 0, 0, 0); // Initial energy in eV, random direction 
        unsigned int nLines = fAvalanche->GetNumberOfElectronEndpoints();

        G4cout << "(Debug: HeedModel.cc) Number of electrons in the avalanche: " << nLines << G4endl;
        for (int i=0; i < nLines; i++) {
            unsigned int n = fAvalanche->GetNumberOfElectronDriftLinePoints(i);
            double xfinal , yfinal, zfinal, tfinal, efinal, e0; // Dummy variables to store the end point of the drift line
            int status; 
            fAvalanche->GetElectronEndpoint(i, x, y, z, t, e0, xfinal, yfinal, zfinal, tfinal, efinal, status); // To get the end point of the drift line
            G4cout << "(Debug: HeedModel.cc) The status of the electron is: " << status << G4endl; 
            if (status == -5) { // If the status is -5, we are inside a wire (see Garfield++ documentation)
                // Which wire did we touch?
                double min_x = 10000.0; 
                int index = -1;
                for (int i = 0; i < nbOfAnodes; i++) {
                  const double xPos = anodesSpacing * (i - nbOfAnodes / 2);
                  if (abs(xPos - xfinal) < min_x) {
                    min_x = abs(xPos - xfinal);
                    index = i;
                  }
                }
                G4cout << "(Debug: HeedModel.cc) The wire touched is: " << index << G4endl;
                electronsInWires[index]++; // Increment the number of electrons in the wire

                secondaryElectronCounter++; // The secondary electron generated by Degrad has touched the wire!  
                G4cout << "(Debug: HeedModel.cc) Secondary electron counter: " << secondaryElectronCounter << G4endl;
            }
            double xi, yi, zi, ti; 

            for (int j=0; j<n; j++){
              fAvalanche->GetElectronDriftLinePoint(xi, yi, zi, ti, j, i); 
              
              if (G4VVisManager::GetConcreteInstance() && j % jumpDriftStepPoints == 0) { // To get all the drift step points or only some of them
                  dlt->AppendStep(G4ThreeVector(xi * CLHEP::cm, yi * CLHEP::cm, zi * CLHEP::cm), ti);
                  G4cout << "(Debug: HeedModel.cc) Appended step: " << xi << " " << yi << " " << zi << G4endl;
              }
            }

        }
        for(int i = 0; i < nbOfAnodes; i++) {
          G4cout << "(Debug: HeedModel.cc) Number of electrons in wire " << i << ": " << electronsInWires[i] << G4endl;
        } 

    } else if (createAval) {
        G4cout << "(Debug: HeedModel.cc) Now drifting an avalanche..." << G4endl;
        fDrift->AvalancheElectron(x, y, z, t);
        unsigned int nLines = fDrift->GetNumberOfElectronEndpoints();
        
        G4cout << "(Debug: HeedModel.cc) Number of electrons in the avalanche: " << nLines << G4endl;
        double x0, y0, z0, t0;
        double xi, yi, zi, ti; // Dummy variables to store the end point of the drift line
        int status;
        for (unsigned int i = 0; i < nLines; i++) {
            fDrift->GetElectronEndpoint(i, x0, y0, z0, t0, xi, yi, zi, ti, status);
            dlt->AppendStep(G4ThreeVector(xi * CLHEP::cm, yi * CLHEP::cm, zi * CLHEP::cm), ti);
            G4cout << "(Debug: HeedModel.cc) Endpoint: " << xi << " " << yi << " " << zi << " " << status << G4endl;
            if (status == -5) { // If the status is -5, we are out of the drift medium (inside a wire) (see Garfield++ documentation)
              secondaryElectronCounter++; // The secondary electron generated by Degrad has touched the wire!  
              G4cout << "(Debug: HeedModel.cc) Secondary electron counter: " << secondaryElectronCounter << G4endl;
          }
        }  

    // If we just set "driftElectrons" to true, then the avalanche is created by default    
    } else {
        G4cout << "(Debug: HeedModel.cc) Now drifting an avalanche..." << G4endl;
        fDrift->DriftElectron(x, y, z, t);
        unsigned int n = fDrift->GetNumberOfIonEndpoints();
        double x0, y0, z0, t0;
        double x1, y1, z1, t1;
        int status;
        for (unsigned int i = 0; i < n; i++) {
            fDrift->GetIonEndpoint(i, x0, y0, z0, t0, x1, y1, z1, t1, status);
            if (G4VVisManager::GetConcreteInstance() && i % jumpDriftStepPoints == 0) {
                dlt->AppendStep(G4ThreeVector(x1 * CLHEP::cm, y1 * CLHEP::cm, z1 * CLHEP::cm), t1);
            }
        }
    }
  }
  G4cout << "(Debug: HeedModel.cc) Number of secondary electrons in this event: " << secondaryElectronCounter << G4endl;
  secondaryElectronCounterTotal += secondaryElectronCounter;   // Total number that have reached the anodes in all the events so far

  numberOfEvents++; // We increase the number of events by one

  // Calculate the aaveraged gas amplification coefficient G
  gasAmplificationCoefficient = double(secondaryElectronCounterTotal) / double(numberOfEvents);

  ProcessEvent(); // Save the signal in the wires to a file
}

// Plot the track, only called when visualization is turned on by the user
void HeedModel::PlotTrack(){

    G4cout << "(Debug: HeedModel.cc) In the PlotTrack method..." << G4endl; 
    G4cout << "(Debug: HeedModel.cc) Number of electrons that have reached the anodes: " << secondaryElectronCounterTotal << G4endl; 
    G4cout << "(Debug: HeedModel.cc) Number of events: " << numberOfEvents << G4endl;
    G4cout << "(Debug: HeedModel.cc) Average gas amplification coefficient G: " << double(secondaryElectronCounterTotal) / double(numberOfEvents) << G4endl;
    
    // Now we plot the data once the simulation has finished
    if (fVisualizeChamber) {
      viewCell->SetArea(-6, -1.5, 6, 1.5); // xmin, ymin, xmax, ymax, in cm
      viewCell->Plot2d(); 
      constexpr bool twod = true; 
      constexpr bool drawaxis = false; 
      viewDrift->Plot(twod, drawaxis); 
      fChamberCanvas->Update();
      fChamberCanvas->Print("HeedDeltaElectronModel_chamber.pdf");
    }
    if (fVisualizeSignal) {
      fSignalCanvas->Clear(); // Clear the signal after each event
      fSensor->PlotSignal("a_5", fSignalCanvas);
      fSignalCanvas->Update();
      fSignalCanvas->Print("HeedDeltaElectronModel_signal.pdf");
    }
    if (fVisualizeField) {
      fFieldCanvas->Clear(); // Clear the field after each event
      viewField->SetNumberOfContours(100);
      viewField->SetArea(-7, -1.5, +7, 1.5);
      viewField->PlotContour("emag");
      fFieldCanvas->Update();
      fFieldCanvas->Print("HeedDeltaElectronModel_efield.pdf");
    }

    // Once we have plotted the data, we reset the parameters for the next shot
    Reset();
}


void HeedModel::ProcessEvent(){
  std::ofstream resultfile("Results_" + nameOfSimulation + ".csv", std::ios::app); // Append mode to avoid overwriting

  // Check if the file is empty (to see if it's the first shot)
  std::ifstream checkfile("Results_" + nameOfSimulation + ".csv");
  bool isEmpty = checkfile.peek() == std::ifstream::traits_type::eof();
  checkfile.close();
  
  if (!resultfile.is_open()) {
      G4cerr << "(Error: HeedModel.cc) Could not open the results file!" << G4endl;
      return;
  }

  // Write the header to the file if it's the first event
  if (isEmpty) {
      resultfile << "Shot Number,Number of events (secondary e.),Anode ID,Electrons in wire,Gas amplification  coefficient\n";
  }

  for(int anodeID = 0; anodeID < nbOfAnodes; anodeID++) {

      // Write the data for this anode to the file
      resultfile << shotNumber << "," // Shot number (constant for a given incoming photon)
      << numberOfEvents << ","  // Event ID
      << anodeID << ","         // Anode ID
      << electronsInWires[anodeID] << ","  // Electrons per wire
      << gasAmplificationCoefficient << "\n" ;// Averaged gas amplification coefficient
  }
               
  // Close the file
  resultfile.close();

  G4cout << "(Debug: HeedModel.cc) Event data saved to Results_" + nameOfSimulation + ".csv" << G4endl;
}

void HeedModel::Reset(){}

void HeedModel::UpdateFromGasModelParameters() {
  // Update voltages
  vAnodeWires = fGasModelParameters->GetVoltageAnodeWires();
  vCathodePlane = fGasModelParameters->GetVoltageCathodePlane();

  G4cout << "(Debug: HeedModel.cc) Updated HeedModel parameters from GasModelParameters." << G4endl;
}
