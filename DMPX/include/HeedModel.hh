/*
 * HeedModel.h
 *
 *  Created on: Apr 9, 2014
 *      Author: dpfeiffe
 */

#ifndef HEEDMODEL_H_
#define HEEDMODEL_H_

// Included from the current project
#include "GasModelParameters.hh"
#include "GasBoxSD.hh"
#include "DetectorMessenger.hh"
#include "DetectorConstruction.hh"

// Included from the loaded libraries (G4, ROOT, Garfield++, Degrad...)
#include "SolidBox.hh"                 //Geometry
#include "SolidTube.hh"                //Geometry
#include "ComponentAnalyticField.hh"    //Garfield field
#include "ViewCell.hh"                  //Visualization
#include "Sensor.hh"
#include "G4ThreeVector.hh"
#include "ComponentVoxel.hh"
#include "AvalancheMicroscopic.hh"
#include "AvalancheMC.hh"
#include "DriftLineRKF.hh"
#include "TCanvas.h"
#include "ViewDrift.hh"
#include "ViewSignal.hh"
#include "ViewField.hh"
#include "ViewGeometry.hh"
#include "G4VFastSimulationModel.hh"
#include "MediumMagboltz.hh"
#include "TrackHeed.hh"
#include "GeometrySimple.hh"


class G4VPhysicalVolume;
class DetectorConstruction;
class HeedMessenger;
class G4FastStep;
class G4FastTrack;

class HeedModel : public G4VFastSimulationModel {
 public:
  //-------------------------
  // Constructor, destructor
  //-------------------------
  HeedModel(GasModelParameters*, G4String, G4Region*, DetectorConstruction*, GasBoxSD*);
  ~HeedModel();

  virtual G4bool IsApplicable(const G4ParticleDefinition&);
  virtual G4bool ModelTrigger(const G4FastTrack&);
  virtual void DoIt(const G4FastTrack&, G4FastStep&);
  
  /*The following public methods are user-dependent*/

  //This method is called after each event, to record the relevant data
  void ProcessEvent();
  //This method is called at the beginning of an event to reset some variables of the class
  void Reset();
  G4bool FindParticleName(G4String name);
  G4bool FindParticleNameEnergy(G4String name,double ekin_keV);

  // Getters from DetectorConstruction, to inherit the same values from the 
  // DetectorMessenger class
  G4double GetAnodesHalfLength(const DetectorConstruction& detCon) {
    const DetectorMessenger& detectorMessenger = *(detCon.GetDetectorMessenger());
    anodesHalfLength = detectorMessenger.GetAnodesHalfLength(); 
    return anodesHalfLength; 
  }

  G4double GetAnodesR(const DetectorConstruction& detCon) {
    const DetectorMessenger& detectorMessenger = *(detCon.GetDetectorMessenger());
    anodesR = detectorMessenger.GetAnodesR();
    return anodesR;  
  }

  G4double GetAnodesSpacing(const DetectorConstruction& detCon) {
    const DetectorMessenger& detectorMessenger = *(detCon.GetDetectorMessenger());
    anodesSpacing = detectorMessenger.GetAnodesSpacing();
    return anodesSpacing; 
  }

  G4int GetNbOfAnodes(const DetectorConstruction& detCon) {
    const DetectorMessenger& detectorMessenger = *(detCon.GetDetectorMessenger());
    nbOfAnodes = detectorMessenger.GetNbOfAnodes();
    return nbOfAnodes; 
  }

  G4String GetNameOfSimulation(const DetectorConstruction& detCon) {
    const DetectorMessenger& detectorMessenger = *(detCon.GetDetectorMessenger());
    nameOfSimulation = detectorMessenger.GetNameOfSimulation();
    return nameOfSimulation; 
  }

  G4double GetCathodes1_LengthX(const DetectorConstruction& detCon) {
    const DetectorMessenger& detectorMessenger = *(detCon.GetDetectorMessenger());
    cathodes_1_LengthX = detectorMessenger.GetCathodes1_LengthX(); 
    return cathodes_1_LengthX; 
  }

  G4double GetCathodes1_LengthY(const DetectorConstruction& detCon) {
    const DetectorMessenger& detectorMessenger = *(detCon.GetDetectorMessenger());
    cathodes_1_LengthY = detectorMessenger.GetCathodes1_LengthY(); 
    return cathodes_1_LengthY; 
  }

  G4double GetCathodes1_LengthZ(const DetectorConstruction& detCon) {
    const DetectorMessenger& detectorMessenger = *(detCon.GetDetectorMessenger());
    cathodes_1_LengthZ = detectorMessenger.GetCathodes1_LengthZ(); 
    return cathodes_1_LengthZ; 
  }

  G4double GetCathodes1_XPos(const DetectorConstruction& detCon) {
    const DetectorMessenger& detectorMessenger = *(detCon.GetDetectorMessenger());
    cathodes_1_XPos = detectorMessenger.GetCathodes1_XPos(); 
    return cathodes_1_XPos; 
  }

  G4double GetCathodes1_ZPos(const DetectorConstruction& detCon) {
    const DetectorMessenger& detectorMessenger = *(detCon.GetDetectorMessenger());
    cathodes_1_ZPos = detectorMessenger.GetCathodes1_ZPos(); 
    return cathodes_1_ZPos; 
  }

  G4double GetGasBoxLengthY(const DetectorConstruction& detCon) {
    const DetectorMessenger& detectorMessenger = *(detCon.GetDetectorMessenger());
    GasBoxLengthY = detectorMessenger.GetGasBoxLengthY(); 
    return GasBoxLengthY; 
  }

  // Setter and getter for the number of events
  void SetNumberOfEvents(G4int n) {
    numberOfEvents = n;
  }

  G4int GetNumberOfEvents() {
    return numberOfEvents;
  }

  void UpdateFromGasModelParameters(); // Method to update member variables from gmp


 protected:
  void InitialisePhysics();
  virtual void Run(G4FastStep& fastStep,const G4FastTrack& fastTrack, G4String particleName, double ekin_keV, double t, double x_cm, double y_cm, double z_cm, double dx, double dy, double dz) = 0;
  void PlotTrack();
  void Drift(double,double, double, double);
  
  DetectorConstruction* detCon;
  HeedMessenger* fHeedMessenger;
  MapParticlesEnergy fMapParticlesEnergy;

  G4String gasFile;
  G4String ionMobFile;
  
  G4double vAnodeWires;
  G4double vCathodePlane;
  G4double vAnodeWires_temp; 
  G4double vCathodePlane_temp; 
  bool driftElectrons;
  bool driftRKF;
  bool trackMicro;
  bool createAval;
  bool fVisualizeChamber;
  bool fVisualizeSignal;
  bool fVisualizeField;


  G4double thermalE; 
  G4double temperature; 
  G4double anodesHalfLength;
  G4double anodesR;
  G4double anodesSpacing;
  G4int nbOfAnodes;
  G4int numberOfEvents; 

  // These are the parameters that matter for calculating the gas amplification coefficient G
  G4int secondaryElectronCounter;
  G4int secondaryElectronCounterTotal;  

  // Optional parameters to speed up the calculation of the drift
  G4int jumpDriftStepPoints;

  Garfield::TrackHeed* fTrackHeed;
  GasBoxSD* fGasBoxSD;

  const char* name;
  
  /*The following private methods and variables are user-dependent*/
 private:
  void makeGas();
  void buildBoxAndField();
  void BuildSensor();
  void SetTracking();
  void SettingChamberView();
  void SettingSignalView();
  void SettingFieldView();

  GasModelParameters* fGasModelParameters;
  DetectorMessenger* detectorMessenger;
  Garfield::MediumMagboltz* fMediumMagboltz;
  Garfield::Sensor* fSensor;
  Garfield::GeometrySimple* geo;
  Garfield::SolidBox* box;
  Garfield::SolidBox* cathodePlane_1;
  Garfield::SolidBox* cathodePlane_2;
  Garfield::SolidTube* wire; 
  Garfield::ComponentAnalyticField* comp;
  Garfield::AvalancheMC* fDrift;
  Garfield::DriftLineRKF* fDriftRKF;
  Garfield::AvalancheMicroscopic* fAvalanche;
  TCanvas* fChamberCanvas;
  TCanvas* fSignalCanvas;
  TCanvas* fFieldCanvas;
  TCanvas* fGeoCanvas;
  Garfield::ViewCell* viewCell;
  Garfield::ViewDrift* viewDrift;
  Garfield::ViewSignal* viewSignal;
  Garfield::ViewField* viewField;
  Garfield::ViewGeometry* geoView; 

  G4double tmin; 
  G4double tstep; 
  G4int nbins;
  G4double gasAmplificationCoefficient; // The averaged amplification coefficient G
  std::vector<int> electronsInWires; // Vector to store the number of electrons in each wire
  G4String nameOfSimulation; // Name used for the storing of results
  G4int shotNumber; 
  G4double distanceAnodeCathodes; 
  G4double cathodes_1_LengthX; 
  G4double cathodes_1_LengthY; 
  G4double cathodes_1_LengthZ; 
  G4double cathodes_1_XPos; 
  G4double cathodes_1_ZPos; 
  G4double GasBoxLengthY; 
};

#endif /* HeedModel_H_ */
