/*
 * HeedDeltaElectronModel.cpp
 *
 *  Created on: Apr 9, 2014
 *      Author: dpfeiffe
 */
#include "../include/HeedDeltaElectronModel.hh"
#include "../include/GasModelParameters.hh"
#include "../include/GasBoxSD.hh"
#include "../include/GasBoxHit.hh"

#include <iostream>
#include "G4VPhysicalVolume.hh"
#include "G4Electron.hh"
#include "G4UnitsTable.hh"
#include "G4Gamma.hh"
#include "G4SystemOfUnits.hh"
#include "G4VVisManager.hh"
#include "G4FastStep.hh"
#include "G4FastTrack.hh"

#include "G4AutoLock.hh"
namespace{G4Mutex aMutex = G4MUTEX_INITIALIZER;}


// HeedDeltaElectronModel derives from the HeedModel Class and uses the GasModelParameters Class to set some user-defined veriables
HeedDeltaElectronModel::HeedDeltaElectronModel(GasModelParameters* gmp,G4String modelName, G4Region* envelope,DetectorConstruction* dc, GasBoxSD* sd)
    : HeedModel(gmp, modelName, envelope, dc, sd) {
        // Particle map
        fMapParticlesEnergy = gmp->GetParticleNamesHeedDeltaElectron();

        // Gas files
        gasFile = gmp->GetGasFile();
        ionMobFile = gmp->GetIonMobilityFile();

        // Voltages
        vAnodeWires = gmp->GetVoltageAnodeWires();
        vCathodePlane = gmp->GetVoltageCathodePlane();

        // Tracking 
        driftElectrons = gmp->GetDriftElectrons();
        driftRKF = gmp->GetDriftRKF();
        trackMicro = gmp->GetTrackMicroscopic();
        createAval = gmp->GetCreateAvalancheMC();

        // Visualization
        fVisualizeChamber = gmp->GetVisualizeChamber();
        fVisualizeSignal = gmp->GetVisualizeSignals();
        fVisualizeField = gmp->GetVisualizeField();

        // Name of the model
        name="HeedDeltaElectronModel";
        InitialisePhysics();

        G4cout << "(Debug: HeedDeltaElectronModel.cc) The HeedDeltaElectronModel has been properly initialized..." << G4endl; 
    }

HeedDeltaElectronModel::~HeedDeltaElectronModel() {}


//This method is called in the DoIt-method in parent class HeedModel
void HeedDeltaElectronModel::Run(G4FastStep& fastStep,const G4FastTrack& fastTrack, G4String particleName, double ekin_eV, double t, double x_cm,
            double y_cm, double z_cm, double dx, double dy, double dz) {

    // G4double ekin_keV = ekin_eV / keV; // For the Transport functions

    G4cout << "(Debug: HeedDeltaElectronModel.cc) The energy here is: " << G4BestUnit(ekin_eV, "Energy") << G4endl; 
    int nc = 0, ni=0; // Number of electrons/ions produced by the delta electron
    G4cout << "(Debug: HeedDeltaElectronModel.cc) Running interface..." << G4endl;
    if(particleName == "e-"){
        G4cout << "(Debug: HeedDeltaElectronModel.cc) Inside the electron case..." << G4endl;
        G4AutoLock lock(&aMutex);
        fTrackHeed->TransportDeltaElectron(x_cm, y_cm, z_cm, t, 
                                           ekin_eV, dx, dy,
                                           dz, nc, ni);
        G4cout << "(Debug: HeedDeltaElectronModel.cc) The number of electrons produced is: " << nc << G4endl;
    }
    else{
        G4AutoLock lock(&aMutex);
        fTrackHeed->TransportPhoton(x_cm, y_cm, z_cm, t, ekin_eV, dx, dy,
                                    dz, nc);
    }
    for (int cl = 0; cl < nc; cl++) {
        double xe, ye, ze, te;
        double ee, dxe, dye, dze;
        fTrackHeed->GetElectron(cl, xe, ye, ze, te, ee, dxe, dye, dze);
        GasBoxHit* gbh = new GasBoxHit();
        gbh->SetPos(G4ThreeVector(xe*CLHEP::cm,ye*CLHEP::cm,ze*CLHEP::cm));
        gbh->SetTime(te);
        fGasBoxSD->InsertGasBoxHit(gbh);

        // If the visManager is on...
        if(G4VVisManager::GetConcreteInstance() && cl % 1 == 0)
            G4cout << "(Debug: HeedDeltaElectronModel.cc) Now drifting..." << G4endl;
            G4cout << "(Debug: HeedDeltaElectronModel.cc) Positions (cm) and time for the drift calculation: " << xe 
            << " " << ye << " " << ze << " " << te << G4endl;
            Drift(xe,ye,ze,te); // Drift from the initial position to the final position in HeedModel
    }
    G4cout << "(Debug: HeedDeltaElectronModel.cc) Now plotting the track..." << G4endl;
    PlotTrack();
    fastStep.KillPrimaryTrack();
    fastStep.ProposePrimaryTrackPathLength(0.0);
    fastStep.ProposeTotalEnergyDeposited(ekin_eV);
}

