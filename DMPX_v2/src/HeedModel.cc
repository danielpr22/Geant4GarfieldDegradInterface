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
  fVisualizeChamber = gmp->GetVisualizeChamber();
  fVisualizeSignal = gmp->GetVisualizeSignals();
  fVisualizeField = gmp->GetVisualizeField();
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

  fastStep.KillPrimaryTrack(); // We first kill the Degrad tracks

  G4cout << "(Debug: HeedModel.cc) The electron has been killed..." << G4endl; 

  G4ThreeVector dir = fastTrack.GetPrimaryTrack()->GetMomentumDirection();

  G4ThreeVector worldPosition = fastTrack.GetPrimaryTrack()->GetPosition();
  G4cout << "(Debug: HeedModel.cc) The position of the particle before drift is: " << G4BestUnit(worldPosition, "Length") << G4endl; 

  G4double ekin = fastTrack.GetPrimaryTrack()->GetKineticEnergy();
  G4double time = fastTrack.GetPrimaryTrack()->GetGlobalTime();
  G4String particleName =
      fastTrack.GetPrimaryTrack()->GetParticleDefinition()->GetParticleName();

  G4cout << "(Debug: HeedModel.cc) Value of ekin: " << G4BestUnit(ekin, "Energy") << G4endl; 

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
  G4cout << "(Debug: HeedModel.cc) The gas file is searched in the Garfield database: " << gasFile << G4endl;
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
  const double anodeSpacing = 0.2; // cm
  const double anodesR = 0.01; // cm
  const double anodesHalfLength = 1.6; // cm

  for (int i = 0; i < nbOfAnodes; i++) {
      const double xPos = anodeSpacing * (i - nbOfAnodes / 2); // cm
      const double yPos = 0.0; // cm
      const double zPos = 0.0; // cm
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
  const double cathodePlaneHalfX = 6.5 ; // cm
  const double cathodePlaneWidth = 0.2; // cm
  const double cathodePlaneHalfZ = 1.6; // cm
  const double xPosPlane = 0.0; // cm
  const double yPosPlane = -1.0; // cm
  const double zPosPlane = 0.0; // cm

  cathodePlane = new Garfield::SolidBox(xPosPlane, yPosPlane, zPosPlane, 
    cathodePlaneHalfX, cathodePlaneWidth, cathodePlaneHalfZ);

  geo->AddSolid(cathodePlane, fMediumMagboltz);
  comp->AddPlaneY(yPosPlane, vCathodePlane, "p"); // Adding the wires for the signal calculation

  G4cout << "(Debug: HeedModel.cc) Added cathode plane to Garfield geometry." << G4endl;

  geoView = new Garfield::ViewGeometry(geo);
  geoView->Plot3d();
  gSystem->ProcessEvents(); // To properly keep on processing the events
}



//Build sensor (see Garfield++ documentation)
void HeedModel::BuildSensor(){
  fSensor = new Garfield::Sensor();
  fSensor->AddComponent(comp);
  fSensor->AddElectrode(comp, "a_18");
  fSensor->SetTimeWindow(0.,10.,1000.); //Lowest time [ns], tstep (signal collected during this time) [ns], tfinal [ns]
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
    fDrift->EnableSignalCalculation(false); // So we can count the number of electrons and not the readout signal
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

//Signal plotting (see Garfield++ documentation)
void HeedModel::SettingSignalView(){
  // fSignalCanvas = new TCanvas("fSignalCanvas", "Signal on the wire", 700, 700);
  // fSensor->PlotSignal("a", fSignalCanvas);
  // fSignalCanvas->RangeAxis(0, -5*1e3, 1000, 5*1e3);

  // viewSignal = new Garfield::ViewSignal();
  // viewSignal->SetSensor(fSensor);
  // viewSignal->SetCanvas(fSignalCanvas);
  // viewSignal->PlotSignal("a");
  // fSignalCanvas->Update(); 
  //fSignalCanvas->Print("HeedDeltaElectronModel_signal.pdf");
}

//Electric field plotting (see Garfield++ documentation)
void HeedModel::SettingFieldView(){
  fFieldCanvas = new TCanvas("fFieldCanvas", "Electric field", 700, 700);
  viewField = new Garfield::ViewField();
  viewField->SetComponent(comp);
  viewField->SetCanvas(fFieldCanvas);
  viewField->SetNumberOfContours(100);
  viewField->SetArea(-7, -2, -4, 2);
  viewField->PlotContour("e");
  fFieldCanvas->Update();
  fFieldCanvas->Print("HeedDeltaElectronModel_efield.pdf");
}

// Drift the electrons from point of creation towards the electrodes (Garfield++ documentation)
void HeedModel::Drift(double x, double y, double z, double t) {
  // Here, we have the point at the START of the drift, and we want to store all the points
  // of the drift calculation
  bool reached_wire = false; 
  secondaryElectronCounter = 0; 

  if (driftElectrons) {
      G4cout << "(Debug: HeedModel.cc) Now drifting an electron..." << G4endl; 
      DriftLineTrajectory* dlt = new DriftLineTrajectory();
      G4TrackingManager* fpTrackingManager = G4EventManager::GetEventManager()->GetTrackingManager();
      fpTrackingManager->SetTrajectory(dlt);

      if (driftRKF) {
        // Starting point of the drift
          G4cout << "(Debug: HeedModel.cc) Starting point of the drift: " << x << " " 
          << y <<  " " << z << " " << t << G4endl; 
          fDriftRKF->DriftElectron(x, y, z, t);
          unsigned int n = fDriftRKF->GetNumberOfDriftLinePoints();
          G4cout << "(Debug: HeedModel.cc) Number of drift line points: " << n << G4endl; 
          double xi, yi, zi, ti;
          int status; // Add a variable to store the status
          for (unsigned int i = 0; i < n; i++) {
              fDriftRKF->GetDriftLinePoint(i, xi, yi, zi, ti); // To get the full trajectory
              if (G4VVisManager::GetConcreteInstance() && i % 1 == 0) { // To get all the drift step points or only some of them
                  dlt->AppendStep(G4ThreeVector(xi * CLHEP::cm, yi * CLHEP::cm, zi * CLHEP::cm), ti);
                  //G4cout << "(Debug: HeedModel.cc) Appended step: " << xi << " " << yi << " " << zi << " " << status << G4endl;
              }
          }
          fDriftRKF->GetEndPoint(xi, yi, zi, ti, status); // Ths command retrieves the endpoint and end time, and the status
          if (status > 0) {
            reached_wire = true; // If the status is > 0, we are inside a wire (see Garfield++ documentation
            secondaryElectronCounter++; // The secondary electron generated by Degrad has touched the wire!  
          }

          G4cout << "(Debug: HeedModel.cc) The secondary electron impacted on one of the anodes?" << " " << reached_wire << G4endl; 

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
  // This is the total number that have reached the anodes in all the events so far
  secondaryElectronCounterTotal += secondaryElectronCounter;
}

// Plot the track, only called when visualization is turned on by the user
void HeedModel::PlotTrack(){

    G4cout << "(Debug: HeedModel.cc) In the PlotTrack method..." << G4endl; 

    G4cout << "(Debug: HeedModel.cc) Number of electrons that have reached the anodes: " << secondaryElectronCounter << G4endl; 
      
    // Now we plot the data once the simulation has finished
    if (fVisualizeChamber) {
      viewCell->SetArea(-6, -2, 6, 2); // xmin, ymin, xmax, ymax, in cm
      viewCell->Plot2d(); 
      constexpr bool twod = true; 
      constexpr bool drawaxis = false; 
      viewDrift->Plot(twod, drawaxis); 
      fChamberCanvas->Update();
      fChamberCanvas->Print("HeedDeltaElectronModel_chamber.pdf");
    }
    if (fVisualizeSignal) {
      fSignalCanvas = new TCanvas("fSignalCanvas", "Signal on the wire", 700, 700);
      fSensor->PlotSignal("a_18", fSignalCanvas);
      fSignalCanvas->Print("HeedDeltaElectronModel_signal.pdf");
    }
}
