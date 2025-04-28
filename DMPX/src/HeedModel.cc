#include <iostream>
#include <stdio.h>

#include "../include/HeedModel.hh"
#include "../include/DetectorConstruction.hh"
#include "../include/DriftLineTrajectory.hh"

#include "G4VPhysicalVolume.hh"
#include "G4Electron.hh"
#include "G4Gamma.hh"
#include "G4SystemOfUnits.hh"
#include "G4RunManager.hh"
#include "G4TrackingManager.hh"
#include "G4EventManager.hh"
#include "G4VVisManager.hh"
#include "G4AutoLock.hh"
#include "TSystem.h" // Including ROOT's TSystem for gSystem

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

/* 
A mutex is a synchronization primitive used to protect shared resources 
from concurrent access by multiple threads.

This construct is used to ensure thread safety in a multithreaded environment
*/

const static G4double torr = 1. / 760. * atmosphere;
namespace{G4Mutex aMutex = G4MUTEX_INITIALIZER;}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

HeedModel::HeedModel(G4String modelName, G4Region* envelope,DetectorConstruction* dc,GasBoxSD* sd)
: G4VFastSimulationModel(modelName, envelope), detCon(dc), fGasBoxSD(sd) {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

HeedModel::~HeedModel() {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//Method called when a particle is created, checks if the model is applicable for this particle
G4bool HeedModel::IsApplicable(const G4ParticleDefinition& particleType) {
  G4String particleName = particleType.GetParticleName();
  return FindParticleName(particleName);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//Method called in every step: checks if the conditions of the particle are met. If true the DoIt-method is called
G4bool HeedModel::ModelTrigger(const G4FastTrack& fastTrack) {
  G4double ekin = fastTrack.GetPrimaryTrack()->GetKineticEnergy();
  G4String particleName =
      fastTrack.GetPrimaryTrack()->GetParticleDefinition()->GetParticleName();
  return FindParticleNameEnergy(particleName, ekin / keV);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//Implementation of the general model, the Run method, called at the end is specifically implemented for the daughter classes
void HeedModel::DoIt(const G4FastTrack& fastTrack, G4FastStep& fastStep) {

  G4ThreeVector dir = fastTrack.GetPrimaryTrack()->GetMomentumDirection();

  G4ThreeVector worldPosition = fastTrack.GetPrimaryTrack()->GetPosition();

  G4double ekin = fastTrack.GetPrimaryTrack()->GetKineticEnergy();
  G4double time = fastTrack.GetPrimaryTrack()->GetGlobalTime();
  G4String particleName =
      fastTrack.GetPrimaryTrack()->GetParticleDefinition()->GetParticleName();

  // The syntax "CLHEP::cm" converts the normal units in Geant4 (mm) to cm, thus dividing the result by 10
  Run(fastStep, fastTrack, particleName, ekin/keV, time, worldPosition.x() / CLHEP::cm,
      worldPosition.y() / CLHEP::cm, worldPosition.z() / CLHEP::cm,
      dir.x(), dir.y(), dir.z());
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//Checks if the particle is in the list of particle for which the model is applicable (called by IsApplicable)
G4bool HeedModel::FindParticleName(G4String name) {
  MapParticlesEnergy::iterator it;
  it = fMapParticlesEnergy.find(name);
  if (it != fMapParticlesEnergy.end()) {
    return true;
  }
  return false;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//Checks if the energy condition of the particle is in the list of conditions for which the model shoould be triggered (called by ModelTrigger)
G4bool HeedModel::FindParticleNameEnergy(G4String name,
                                             double ekin_keV) {
  MapParticlesEnergy::iterator it;
//  it = fMapParticlesEnergy->find(name);
  for (it=fMapParticlesEnergy.begin(); it!=fMapParticlesEnergy.end();++it) {
    if(it->first == name){
      EnergyRange_keV range = it->second;
      if (range.first <= ekin_keV && range.second >= ekin_keV) {
        return true;
      }
    }
  }
  return false;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//Initialize the Garfield++ related geometries and physics/tracking mechanisms, this is specific for each use and should be re-implemented entirely
void HeedModel::InitialisePhysics() {
  // This condition evaluates whether the simulation is running in a worker thread (in a multithreaded environment) or in a sequential mode (single-threaded environment).
  if(G4RunManager::GetRunManager()->GetRunManagerType() == G4RunManager::workerRM || G4RunManager::GetRunManager()->GetRunManagerType() == G4RunManager::sequentialRM){
    G4cout << "(Debug: HeedModel.cc) Initializing the physics of the Heed model..." << G4endl;
    
    makeGas();
      
    buildBox();
    
    BuildCompField();
    
    BuildSensor();
    
    SetTracking();
    
    if(fVisualizeChamber) CreateChamberView();
    if(fVisualizeSignal) CreateSignalView();
    if(fVisualizeField) CreateFieldView();
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// Gas intialization (see Garfield++ documentation)
void HeedModel::makeGas() {
  fMediumMagboltz = new Garfield::MediumMagboltz();
  double pressure = detCon->GetGasPressure()/torr;
  double temperature = detCon->GetTemperature()/kelvin;
  double kryptonPerc = detCon->GetKryptonPercentage();
  double ch4Perc = detCon->GetCH4Percentage();
  fMediumMagboltz->SetComposition("kr", kryptonPerc, "ch4", ch4Perc);
  fMediumMagboltz->SetTemperature(temperature);
  fMediumMagboltz->SetPressure(pressure); 
  fMediumMagboltz->EnableDebugging();
  fMediumMagboltz->Initialise(true);
  fMediumMagboltz->DisableDebugging();

  G4cout << gasFile << G4endl;
  const std::string path = getenv("GARFIELD_HOME");
  G4AutoLock lock(&aMutex);
  if(ionMobFile!="")
    fMediumMagboltz->LoadIonMobility(path + "/Data/" + ionMobFile);
  if(gasFile!="")
      fMediumMagboltz->LoadGasFile(gasFile.c_str());
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
  
//Geometry (see Garfield++ documentation)
void HeedModel::buildBox() {
  geo = new Garfield::GeometrySimple();
  
  // We build the gas box for the DMPX
  box = new Garfield::SolidBox((detCon->GetGasBoxCenterPositionX())/CLHEP::mm, (detCon->GetGasBoxCenterPositionY())/CLHEP::mm,
  (detCon->GetGasBoxCenterPositionZ())/CLHEP::mm, (detCon->GetGasBoxLengthX() * 0.5)/ CLHEP::mm, 
  (detCon->GetGasBoxLengthY() * 0.5) / CLHEP::mm, (detCon->GetGasBoxLengthZ() * 0.5) / CLHEP::mm);

  // Debugging messages for the gas box
  G4cout << "(Debug: HeedModel.cc) The length of the gas box in the X direction is: " << detCon->GetGasBoxLengthX() << " mm" << G4endl;
  G4cout << "(Debug: HeedModel.cc) The length of the gas box in the Y direction is: " << detCon->GetGasBoxLengthY() << " mm" << G4endl;
  G4cout << "(Debug: HeedModel.cc) The length of the gas box in the Z direction is: " << detCon->GetGasBoxLengthZ() << " mm" << G4endl;
  G4cout << "(Debug: HeedModel.cc) The center position of the gas box in the X direction is: " << detCon->GetGasBoxCenterPositionX() << " mm" << G4endl;
  G4cout << "(Debug: HeedModel.cc) The center position of the gas box in the Y direction is: " << detCon->GetGasBoxCenterPositionY() << " mm" << G4endl;
  G4cout << "(Debug: HeedModel.cc) The center position of the gas box in the Z direction is: " << detCon->GetGasBoxCenterPositionZ() << " mm" << G4endl;
  G4cout << "(Debug: HeedModel.cc) The pressure of the gas is: " << detCon->GetGasPressure() / torr << " torr" << G4endl;
  G4cout << "(Debug: HeedModel.cc) The temperature of the gas is: " << detCon->GetTemperature() / kelvin << " K" << G4endl;
  G4cout << "(Debug: HeedModel.cc) The gas is made of " << detCon->GetKryptonPercentage() << "% Kr and " << detCon->GetCH4Percentage() << "% CH4" << G4endl;
  geo->AddSolid(box, fMediumMagboltz);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//Construction of the electric field (see Garfield++ documentation)
void HeedModel::BuildCompField() {

  // y-axis gap between rows of wires [cm]
  // Equivalent to: condition ? value_if_true : value_if_false;
  const double gap = 0.2;
  
  // y coordinates of the wires [cm]
  const double ys = gap;            // anode wires
  const double yc = 2. * gap;       // cathode
  const double yg = 2. * gap + 0.3; // gate

  // Periodicity (wire spacing)
  const double period = 0.25;
  const int nRep = 2;
  
  const double dc = period;
  const double dg = period / 2;
  
  // Wire diameters [cm]
  const double dSens = 0.0020;
  const double dCath = 0.0075;
  const double dGate = 0.0075;
  
  comp = new Garfield::ComponentAnalyticField();
  comp->SetGeometry(geo);
  
  comp->SetPeriodicityX(nRep * period);

  // Debug comments for the geometry of the wires
  G4cout << "(Debug: HeedModel.cc) The periodicity of the wires is: " << nRep * period << " cm" << G4endl;
  G4cout << "(Debug: HeedModel.cc) The gap between the wires is: " << gap << " cm" << G4endl;
  G4cout << "(Debug: HeedModel.cc) The y coordinate of the anode wires is: " << ys << " cm" << G4endl;
  G4cout << "(Debug: HeedModel.cc) The y coordinate of the cathode wires is: " << yc << " cm" << G4endl;

  // For the anodes
  for (int i = 0; i < nRep; ++i) {
      comp->AddWire((i - 1) * period, (detCon->GetGasBoxLengthZ()*0.5)/CLHEP::mm - ys, dSens, vAnodeWires, "s");
  }
  // For the cathodes
  for (int i = 0; i < nRep; ++i) {
      comp->AddWire(dc * (i - 0.5),(detCon->GetGasBoxLengthZ()*0.5)/CLHEP::mm - yc, dCath, vCathodeWires, "c");
  }
  // For the gate wires
  for (int i = 0; i < nRep * 2; ++i) {
      const double xg = dg * (i - 1.5);
      comp->AddWire(xg,(detCon->GetGasBoxLengthZ()*0.5)/CLHEP::cm - yg, dGate, vGate, "g", 100., 50., 19.3, 1);
  }
  // Add the planes.
  comp->AddPlaneY((detCon->GetGasBoxLengthZ()*0.5)/CLHEP::cm, vPlaneLow, "pad_plane");
  comp->AddPlaneY(-(detCon->GetGasBoxLengthZ()*0.5)/CLHEP::cm, vPlaneHV, "HV");    
  
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//Build sensor (see Garfield++ documentation)
void HeedModel::BuildSensor() {
  fSensor = new Garfield::Sensor();
  fSensor->AddComponent(comp);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//Set which tracking mechanism to be used: Runge-kutta, Monte-Carlo or Microscopic (see Garfield++ documentation)
void HeedModel::SetTracking() {
  if(driftRKF){
    fDriftRKF = new Garfield::DriftLineRKF();
    fDriftRKF->SetSensor(fSensor);
    fDriftRKF->EnableDebugging();
  }
  else if(trackMicro){
    fAvalanche = new Garfield::AvalancheMicroscopic();
    fAvalanche->SetSensor(fSensor);
    fAvalanche->EnableSignalCalculation();
  }
  else{  
    fDrift = new Garfield::AvalancheMC();
    fDrift->SetSensor(fSensor);
    fDrift->EnableSignalCalculation();
    fDrift->SetDistanceSteps(2.e-3);
    if(createAval) fDrift->EnableAttachment();
    else fDrift->DisableAttachment();
  }
  fTrackHeed = new Garfield::TrackHeed();
  fTrackHeed->SetSensor(fSensor);
  fTrackHeed->SetParticle("e-");
  fTrackHeed->EnableDeltaElectronTransport();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// Set some visualization variables to see tracks and drift lines (see Garfield++ documentation)
void HeedModel::CreateChamberView() {
  char str[30];
  strcpy(str,name);
  strcat(str,"_chamber");
  fChamber = new TCanvas(str, "Chamber View", 700, 700);
  cellView = new Garfield::ViewCell();
  cellView->SetComponent(comp);
  cellView->SetCanvas(fChamber);
  cellView->Plot2d();
  fChamber->Update();
  char str2[30];
  strcpy(str2,name);
  strcat(str2,"_chamber.pdf");
  fChamber->Print(str2);
  gSystem->ProcessEvents();
  std::cout << "CreateCellView()" << std::endl;
  
  viewDrift = new Garfield::ViewDrift();
  viewDrift->SetCanvas(fChamber);
  if(driftRKF) fDriftRKF->EnablePlotting(viewDrift);
  else if(trackMicro) fAvalanche->EnablePlotting(viewDrift);
  else fDrift->EnablePlotting(viewDrift);
  fTrackHeed->EnablePlotting(viewDrift);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//Signal plotting (see Garfield++ documentation)
void HeedModel::CreateSignalView() {
  char str[30];
  strcpy(str,name);
  strcat(str,"_signal");
  fSignal = new TCanvas(str, "Signal on the wire", 700, 700);
  viewSignal = new Garfield::ViewSignal();
  viewSignal->SetSensor(fSensor);
  viewSignal->SetCanvas(fSignal);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//Electric field plotting (see Garfield++ documentation)
void HeedModel::CreateFieldView() {
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

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// Drift the electrons from point of creation towards the electrodes (This is common for both models, i.e. HeedDeltaElectron and HeedModel) (see Garfield++ documentation)
void HeedModel::Drift(double x, double y, double z, double t) {
  if (driftElectrons) {
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

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// Plot the track, only called when visualization is turned on by the user
void HeedModel::PlotTrack(){
    if(fVisualizeChamber){
      G4cout << "(Debug: HeedModel.cc) Plotting with PlotTrack..." << G4endl;
      viewDrift->Plot(true,false);
      fChamber->Update();
      fChamber->Print("PrimaryTrack.pdf");
    }
}
