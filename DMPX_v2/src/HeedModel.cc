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
  double temperature = 293.15 * kelvin; // 20 degrees Celsius
  double krPerc = detCon->GetKryptonPercentage();
  double ch4Perc = detCon->GetCH4Percentage();
  fMediumMagboltz->SetComposition("ar", 93., "co2", 7.);
  fMediumMagboltz->SetPressure(pressure); 
  fMediumMagboltz->SetTemperature(temperature);

  G4cout << "(Debug: HeedModel.cc) Now writing the gas file..." << G4endl;

  // ####################
  // ##### MAGBOLTZ #####
  // ####################

  // Please, comment out the following lines if you do not want to use Magboltz to 
  // generate the gas file. The gas file is generated only once, and then it is used
  // for the rest of the simulation.

  // // Generating the gas file for the current gas mixture
  // // SetFieldGrid(Min electric field [V/cm], max electric field [V/cm], number of points in the field grid, 
  // // and whether to use log spacing)
  // fMediumMagboltz->SetFieldGrid(100.0, 100.e3, 20., true); 

  // // Number of collisions (in multiples of 10^7) over which the electron is traced by Magboltz
  // const int ncoll = 10; 

  // // Generating the gas table (.gas file)
  // fMediumMagboltz->GenerateGasTable(ncoll);

  // /// we save the generated file for later use
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
  box = new Garfield::SolidBox(0.0, 0.0, 0.0, // Center coordinates (x, y, z)
                               detCon->GetGasBoxLengthX()*0.5/CLHEP::cm,
                               detCon->GetGasBoxLengthY()*0.5/CLHEP::cm,
                               detCon->GetGasBoxLengthZ()*0.5/CLHEP::cm);

  geo->AddSolid(box, fMediumMagboltz);
  G4cout << "(Debug: HeedModel.cc) Added gas box to Garfield geometry." << G4endl;

  // Creating the component analytic field for the field calculation
  comp = new Garfield::ComponentAnalyticField();
  comp->SetGeometry(geo);

  // Omitting the plane for the moment
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
  const double yPosPlane = 1.0; // cm
  const double zPosPlane = 0.0; // cm

  cathodePlane = new Garfield::SolidBox(xPosPlane, yPosPlane, zPosPlane, 
    cathodePlaneHalfX, cathodePlaneWidth, cathodePlaneHalfZ);

  geo->AddSolid(cathodePlane, fMediumMagboltz);
  comp->AddPlaneY(yPosPlane, vCathodePlane, "p"); // Adding the wires for the signal calculation

  G4cout << "(Debug: HeedModel.cc) Added cathode plane to Garfield geometry." << G4endl;

  // Additional commands for the visualization of the geometry in Garfield++
  geoView = new Garfield::ViewGeometry(geo);
  geoView->Plot3d();
  gSystem->ProcessEvents(); // To properly keep on processing the events
}



//Build sensor (see Garfield++ documentation)
void HeedModel::BuildSensor(){
  fSensor = new Garfield::Sensor();
  fSensor->AddComponent(comp);
  fSensor->AddElectrode(comp, "a_18");
  //fSensor->SetTimeWindow(0.,10.,1000.); //Lowest time [ns], tstep (signal collected during this time) [ns], tfinal [ns]
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

    //fAvalanche->EnableNullCollisionSteps(true); 
    //fAvalanche->EnableDebugging();
  }
  else {  
    fDrift = new Garfield::AvalancheMC();
    fDrift->SetSensor(fSensor);
    fDrift->EnableSignalCalculation(); // So we can count the number of electrons and not the readout signal
    fDrift->SetDistanceSteps(1.e-4); // Step size for the drift line set to 1 micron
    G4cout << "(Debug: HeedModel.cc) The avalanche is being created..." << G4endl;
    if(createAval) {
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
  bool reached_wire = false; 
  secondaryElectronCounter = 0; 

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
      
      // If we want microscopic tracking of the secondary electrons
      } else if (trackMicro) {
          G4cout << "(Debug: HeedModel.cc) Now drifting an avalanche..." << G4endl;
          fAvalanche->AvalancheElectron(x, y, z, t, 10., 0, 0, 0); // Initial energy in eV
          int ne, ni; 
          fAvalanche->GetAvalancheSize(ne, ni); // Number of electrons and ions in the avalanche
          G4cout << "(Debug: HeedModel.cc) Number of electrons in the avalanche: " << ne << G4endl;
          G4cout << "(Debug: HeedModel.cc) Number of ions in the avalanche: " << ni << G4endl;
          // Loop over the electrons in the avalanche
          for (const auto& electron : fAvalanche->GetElectrons()) {
              // Initial position
              const auto& p0 = electron.path[0];
              G4cout << "(Debug: HeedModel.cc) Electron initial position: " << p0.x << " "
              << p0.y << " " << p0.z << " " << p0.t << G4endl;
              
              // Final position
              const auto& p1 = electron.path[electron.path.size() - 1];
              G4cout << "(Debug: HeedModel.cc) Electron final position: " << p1.x << " "
              << p1.y << " " << p1.z << " " << p1.t << G4endl;

              // Status code
              G4cout << "(Debug: HeedModel.cc) Electron status code: " << electron.status << G4endl;
          }
          // unsigned int nLines = fAvalanche->GetNumberOfElectronEndpoints(); 
          // G4cout << "(Debug: HeedModel.cc) Number of electron endpoints: " << nLines << G4endl;

          // double x0, y0, z0, t0;
          // double xi, yi, zi, ti;
          // double dx, dy, dz, dt; 
          // double v, ekin; 
          // for (unsigned int i = 0; i < nLines; i++) {
          //     unsigned int n = fAvalanche->GetNumberOfElectronDriftLinePoints(i);
          //     G4cout << "(Debug: HeedModel.cc) Number of drift lines: " << n << G4endl;

          //     fAvalanche->GetElectronDriftLinePoint(x0, y0, z0, t0, 0, i); // We get the starting point of the drift line
          //     for (unsigned int j = 1; j < n; j++) {
          //         fAvalanche->GetElectronDriftLinePoint(xi, yi, zi, ti, j, i);
          //         dx = (xi - x0) / m; // We convert to SI units
          //         dy = (yi - y0) / m;
          //         dz = (zi - z0) / m;
          //         dt = (ti - t0) / s;
          //         x0 = xi; // We upload the new coordinates and time
          //         y0 = yi;
          //         z0 = zi;
          //         t0 = ti;
          //         G4cout << "(Debug: HeedModel.cc) Difference in distance: " << G4BestUnit(sqrt(dx * dx + dy * dy + dz * dz), "Length") << G4endl;

          //         v = sqrt(dx * dx + dy * dy + dz * dz) / dt;
          //         ekin = 0.5 * 9.10938356e-31 * v * v / (1.602176634e-19); // Kinetic energy in eV
          //         if (G4VVisManager::GetConcreteInstance() && j % 1 == 0) {
          //             dlt->AppendStep(G4ThreeVector(xi * CLHEP::cm, yi * CLHEP::cm, zi * CLHEP::cm), ti);
          //             G4cout << "(Debug: HeedModel.cc) Appended step: " << xi << " " << yi << " " << zi << " " << G4BestUnit(ekin, "Energy") << G4endl;
          //         }
          //     }

      // If we want to use Monte-Carlo methods for the simulation of the avalanches created
      // by the secondary electrons. It is this function that allows for example to recover
      // the gas amplification factor G (average number of electrons created per secondary
      // electron generated by Degrad)
      } else if (createAval) {
          G4cout << "(Debug: HeedModel.cc) Now drifting an avalanche..." << G4endl;
          fTrackHeed->NewTrack(x, y, z, t, 0., 0., 0.); // The initial position of the delta electron and a random direction

          for (const auto& cluster : fTrackHeed->GetClusters()) {
              G4cout << "(Debug: HeedModel.cc) Now in a cluster..." << G4endl;

              for (const auto& electron : cluster.electrons) {
                  // We simulate the electron drift lines
                  fDrift->DriftElectron(electron.x, electron.y, electron.z, electron.t);
              }
          }
          // fDrift->AvalancheElectronHole(x, y, z, t);
          // unsigned int n = fDrift->GetNumberOfElectronEndpoints();
          // G4cout << "(Debug: HeedModel.cc) Number of electron endpoints: " << n << G4endl;
          // double x0, y0, z0, t0;
          // double xi, yi, zi, ti;
          // int status;
          // for (unsigned int i = 0; i < n; i++) {
          //     fDrift->GetElectronEndpoint(i, x0, y0, z0, t0, xi, yi, zi, ti, status);
          //     if (G4VVisManager::GetConcreteInstance() && i % 1 == 0) {
          //         dlt->AppendStep(G4ThreeVector(xi * CLHEP::cm, yi * CLHEP::cm, zi * CLHEP::cm), ti);
          //         G4cout << "(Debug: HeedModel.cc) Appended step: " << xi << " " << yi << " " << zi << " " << G4endl;
          //     }
          // }   

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
      viewCell->SetArea(-6, -3, 6, 4); // xmin, ymin, xmax, ymax, in cm
      viewCell->Plot2d(); 
      constexpr bool twod = true; 
      constexpr bool drawaxis = false; 
      viewDrift->Plot(twod, drawaxis); 
      fChamberCanvas->Update();
      fChamberCanvas->Print("HeedDeltaElectronModel_chamber.pdf");
    }
    if (fVisualizeSignal) {
      fSensor->PlotSignal("a_18", fSignalCanvas);
      fSignalCanvas->Update();
      fSignalCanvas->Print("HeedDeltaElectronModel_signal.pdf");
    }
    if (fVisualizeField) {
      viewField->SetNumberOfContours(100);
      viewField->SetArea(-7, -3, +7, 3);
      viewField->PlotContour("e");
      fFieldCanvas->Update();
      fFieldCanvas->Print("HeedDeltaElectronModel_efield.pdf");
    }
}
