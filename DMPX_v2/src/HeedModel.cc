#include <iostream>
#include <stdio.h>

#include "../include/HeedModel.hh"
#include "../include/DriftLineTrajectory.hh"
#include "../include/DetectorConstruction.hh"

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
#include <TSystem.h> // Include ROOT's TSystem header for gSystem

namespace{G4Mutex aMutex = G4MUTEX_INITIALIZER;}

const static G4double torr = 1. / 760. * atmosphere;

HeedModel::HeedModel(GasModelParameters* gmp, G4String modelName, G4Region* envelope,DetectorConstruction* dc,GasBoxSD* sd)
: G4VFastSimulationModel(modelName, envelope), detCon(dc), fGasBoxSD(sd) {
  thermalE=gmp->GetThermalEnergy();
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
  if (ekin<thermalE) {
		G4cout << "(Debug: HeedModel.cc) Triggered! The Garfield model is triggered below energies of " <<  G4BestUnit(thermalE, "Energy") << G4endl;
		return true;
  }
  else {return false;} 
}

//Implementation of the general model, the Run method, called at the end, is specifically implemented for the daughter classes
void HeedModel::DoIt(const G4FastTrack& fastTrack, G4FastStep& fastStep) {

  G4cout << "(Debug: HeedModel.cc) Inside the DoIt method of HeedModel..." << G4endl; 

  G4ThreeVector dir = fastTrack.GetPrimaryTrack()->GetMomentumDirection();

  G4ThreeVector worldPosition = fastTrack.GetPrimaryTrack()->GetPosition();
  G4cout << "(Debug: HeedModel.cc) The position of the particle before drift is: " << G4BestUnit(worldPosition, "Length") << G4endl; 

  G4double ekin = fastTrack.GetPrimaryTrack()->GetKineticEnergy();
  G4double time = fastTrack.GetPrimaryTrack()->GetGlobalTime();
  G4String particleName =
      fastTrack.GetPrimaryTrack()->GetParticleDefinition()->GetParticleName();

  G4cout << "(Debug: HeedModel.cc) Value of ekin: " << G4BestUnit(ekin, "Energy") << G4endl; 

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
      if (range.first <= ekin_keV && range.second >= ekin_keV) {
        return true;
      }
    }
  }
  return false;
}

// Initialize the Garfield++ related geometries and physics/tracking mechanisms, this is specific for each use and should be re-implemented entirely
// These functions should mimick the situation represented by Geant4
void HeedModel::InitialisePhysics(){
  if(G4RunManager::GetRunManager()->GetRunManagerType() == G4RunManager::workerRM || G4RunManager::GetRunManager()->GetRunManagerType() == G4RunManager::sequentialRM){
    makeGas();
      
    buildBoxAndField();
        
    BuildSensor();
    
    SetTracking();
    
    if(fVisualizeChamber) CreateChamberView();
    if(fVisualizeSignal) CreateSignalView();
    if(fVisualizeField) CreateFieldView();

    G4cout << "(Debug: HeedModel.cc) The HeedModel has been initialized..." << G4endl; 
  }
}

// Gas intialization (see Garfield++ documentation)
void HeedModel::makeGas(){
  fMediumMagboltz = new Garfield::MediumMagboltz();
  double pressure = detCon->GetGasPressure()/torr;
  double temperature = detCon->GetTemperature()/kelvin;
  double krPerc = detCon->GetKryptonPercentage();
  double ch4Perc = detCon->GetCH4Percentage();
  fMediumMagboltz->SetComposition("kr", krPerc, "ch4", ch4Perc);
  fMediumMagboltz->SetTemperature(temperature);
  fMediumMagboltz->SetPressure(pressure); 
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
  G4cout << "(Debug: HeedModel.cc) The gas file is searched in the Garfield database: " << ionMobFile << G4endl;
  fMediumMagboltz->LoadGasFile(gasFile.c_str());
}
  


//Geometry (see Garfield++ documentation)
void HeedModel::buildBoxAndField(){

  geo = new Garfield::GeometrySimple();

  // We must be careful with the coordinates in Garfield++ and Geant4 : the z coordinate in 
  // Garfield++ corresponds to the y coordinate in Geant4!
  box = new Garfield::SolidBox(0.0, 0.0, 0.0, // Center coordinates (x, y, z)
                               detCon->GetGasBoxLengthX()*0.5/CLHEP::cm,
                               detCon->GetGasBoxLengthY()*0.5/CLHEP::cm,
                               detCon->GetGasBoxLengthZ()*0.5/CLHEP::cm);

  geo->AddSolid(box, fMediumMagboltz);
  G4cout << "(Debug: HeedModel.cc) Added gas box to Garfield geometry." << G4endl;

  // Creating the component analytic field for the field calculation
  comp = new Garfield::ComponentAnalyticField();
  comp->SetGeometry(geo);

  // Creating the anode wires geometry
  const int nbOfAnodes = 64; 
  const double anodeSpacing = 2.0; // mm
  const double anodesR = 0.1; // mm
  const double anodesHalfLength = 16; // mm

  for (int i = 0; i < nbOfAnodes; i++) {
      const double xPos = anodeSpacing * (i - nbOfAnodes / 2); // mm
      const double yPos = 0.0; // mm
      const double zPos = 0.0; // mm
      wire = new Garfield::SolidTube(xPos / 10.0, // convert mm to cm (Garfield units)
        yPos, 
        zPos, 
        anodesR / 10.0, // radius in cm
        anodesHalfLength / 10.0 // in cm
      ); 
      geo->AddSolid(wire, fMediumMagboltz);
      comp->AddWire(xPos, yPos, anodesR / 10.0, vAnodeWires, "a"); // Adding the wires for the signal calculation
  }

  G4cout << "(Debug: HeedModel.cc) All the wires have been added to the Garfield geometry...." << G4endl;

  // Creating the cathode plane geometry
  const double cathodePlaneHalfX = 65.0 ; // mm
  const double cathodePlaneWidth = 2.0; // mm
  const double cathodePlaneHalfZ = 16.0; // mm
  const double xPosPlane = 0.0; // mm
  const double yPosPlane = -50.0; // mm
  const double zPosPlane = 0.0; // mm

  cathodePlane = new Garfield::SolidBox(xPosPlane / 10.0, yPosPlane / 10.0, zPosPlane / 10.0, 
    cathodePlaneHalfX / 10.0, cathodePlaneWidth / 10.0, cathodePlaneHalfZ / 10.0); // convert mm to cm (Garfield units)

  geo->AddSolid(cathodePlane, fMediumMagboltz);
  comp->AddPlaneY(yPosPlane, vCathodePlane, "p"); // Adding the wires for the signal calculation

  G4cout << "(Debug: HeedModel.cc) Added cathode plane to Garfield geometry." << G4endl;

  geoView = new Garfield::ViewGeometry(geo);
  geoView->Plot3d();
  //gSystem->ProcessEvents(); // To properly keep on processing the events
}



//Build sensor (see Garfield++ documentation)
void HeedModel::BuildSensor(){
  fSensor = new Garfield::Sensor();
  fSensor->AddComponent(comp);
  fSensor->SetTimeWindow(0.,1000.,10000); //Lowest time [ns], time bins [ns], number of bins
}

//Set which tracking mechanism to be used: Runge-kutta, Monte-Carlo or Microscopic (see Garfield++ documentation)
void HeedModel::SetTracking(){
  if(driftRKF) {
    fDriftRKF = new Garfield::DriftLineRKF();
    fDriftRKF->SetSensor(fSensor);
    fDriftRKF->EnableDebugging();
  }
  else if(trackMicro) {
    fAvalanche = new Garfield::AvalancheMicroscopic();
    fAvalanche->SetSensor(fSensor);
    fAvalanche->EnableSignalCalculation();
  }
  else {  
    fDrift = new Garfield::AvalancheMC();
    fDrift->SetSensor(fSensor);
    fDrift->EnableSignalCalculation();
    fDrift->SetDistanceSteps(2.e-3);
    G4cout << "(Debug: HeedModel.cc) The avalanche is being created..." << G4endl;
    if(createAval) fDrift->EnableAttachment();
    else fDrift->DisableAttachment();
  }
  fTrackHeed = new Garfield::TrackHeed();
  fTrackHeed->SetSensor(fSensor);
  fTrackHeed->SetParticle("e-");
  fTrackHeed->EnableDeltaElectronTransport();
}

// Set some visualization variables to see tracks and drift lines (see Garfield++ documentation)
void HeedModel::CreateChamberView(){
  TCanvas* fChamber = new TCanvas("Chamber", "Chamber View", 800, 700);
  Garfield::ViewCell* viewCell = new Garfield::ViewCell();
  viewCell->SetCanvas(fChamber);
  viewCell->SetComponent(comp); // Setting the ComponentAnalyticField
  viewCell->Plot2d();
  fChamber->Update();
  //gSystem->ProcessEvents(); // Ensure the ROOT GUI processes events
  G4cout << "(Debug: HeedModel.cc) Creating chamber view..." << G4endl;
  viewDrift = new Garfield::ViewDrift();
  viewDrift->SetCanvas(fChamber);
  if(driftRKF) fDriftRKF->EnablePlotting(viewDrift);
  else if(trackMicro) fAvalanche->EnablePlotting(viewDrift);
  else fDrift->EnablePlotting(viewDrift);
  fTrackHeed->EnablePlotting(viewDrift);
}

//Signal plotting (see Garfield++ documentation)
void HeedModel::CreateSignalView(){
  fSignal = new TCanvas("Signal", "Signal on the wire", 700, 700);
  viewSignal = new Garfield::ViewSignal();
  viewSignal->SetSensor(fSensor);
  viewSignal->SetCanvas(fSignal);
}

//Electric field plotting (see Garfield++ documentation)
void HeedModel::CreateFieldView(){
  char str[30];
  strcpy(str,name);
  strcat(str,"_efield");
  fField = new TCanvas(name, "Electric field", 700, 700);
  viewField = new Garfield::ViewField();
  viewField->SetCanvas(fField);
  viewField->SetComponent(comp);
  viewField->SetNumberOfContours(40);
  viewField->PlotContour("e");
  fField->Update();
  char str2[30];
  strcpy(str2,name);
  strcat(str2,"_efield.pdf");
  fField->Print(str2);
}

// Drift the electrons from point of creation towards the electrodes (This is common for both models, i.e. HeedDeltaElectron and HeedModel) (see Garfield++ documentation)
void HeedModel::Drift(double x, double y, double z, double t) {
  if (driftElectrons) {
      G4cout << "(Debug: HeedModel.cc) Now drifting an electron..." << G4endl; 
      DriftLineTrajectory* dlt = new DriftLineTrajectory();
      G4TrackingManager* fpTrackingManager = G4EventManager::GetEventManager()->GetTrackingManager();
      fpTrackingManager->SetTrajectory(dlt);

      if (driftRKF) {
          fDriftRKF->DriftElectron(x, y, z, t);
          unsigned int n = fDriftRKF->GetNumberOfDriftLinePoints();
          double xi, yi, zi, ti;
          int status; // Add a variable to store the status
          for (unsigned int i = 0; i < n; i++) {
              fDriftRKF->GetEndPoint(xi, yi, zi, ti, status); // Pass correct arguments
              if (G4VVisManager::GetConcreteInstance() && i % 1000 == 0) {
                  dlt->AppendStep(G4ThreeVector(xi * CLHEP::cm, yi * CLHEP::cm, zi * CLHEP::cm), ti);
                  G4cout << "(Debug: HeedModel.cc) Appended step: " << xi << " " << yi << " " << zi << G4endl;
              }
          }
      } else if (trackMicro) {
          fAvalanche->AvalancheElectron(x, y, z, t, 0, 0, 0, 0);
          unsigned int nLines = fAvalanche->GetNumberOfElectronEndpoints();
          for (unsigned int i = 0; i < nLines; i++) {
              unsigned int n = fAvalanche->GetNumberOfElectronDriftLinePoints(i);
              double xi, yi, zi, ti;
              for (unsigned int j = 0; j < n; j++) {
                  fAvalanche->GetElectronDriftLinePoint(xi, yi, zi, ti, j, i);
                  if (G4VVisManager::GetConcreteInstance() && j % 1000 == 0) {
                      dlt->AppendStep(G4ThreeVector(xi * CLHEP::cm, yi * CLHEP::cm, zi * CLHEP::cm), ti);
                  }
              }
          }
      } else {
          fDrift->DriftElectron(x, y, z, t);
          unsigned int n = fDrift->GetNumberOfIonEndpoints();
          double x0, y0, z0, t0;
          double x1, y1, z1, t1;
          int status;
          for (unsigned int i = 0; i < n; i++) {
              fDrift->GetIonEndpoint(i, x0, y0, z0, t0, x1, y1, z1, t1, status);
              if (G4VVisManager::GetConcreteInstance() && i % 1000 == 0) {
                  dlt->AppendStep(G4ThreeVector(x1 * CLHEP::cm, y1 * CLHEP::cm, z1 * CLHEP::cm), t1);
              }
          }
      }
  }
}

// Plot the track, only called when visualization is turned on by the user
void HeedModel::PlotTrack(){
    if(fVisualizeChamber){
      G4cout << "(Debug: HeedModel.cc) Plotting track..." << G4endl;
      viewDrift->Plot(true,false);
      fChamber->Update();
      fChamber->Print("PrimaryTrack.pdf");
    }
    if (fVisualizeSignal) {
      G4cout << "(Debug: HeedModel.cc) Now plotting the signal..." << G4endl; 
      viewSignal->PlotSignal("Signal", "h");
      fSignal->Update();
      fSignal->Print("Signal.pdf"); 
    }
}
