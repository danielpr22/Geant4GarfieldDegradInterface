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
    : HeedModel(gmp, modelName, envelope,dc,sd) {
        fMapParticlesEnergy = gmp->GetParticleNamesHeedDeltaElectron();
        gasFile = gmp->GetGasFile();
        ionMobFile = gmp->GetIonMobilityFile();
        driftElectrons = gmp->GetDriftElectrons();
        trackMicro = gmp->GetTrackMicroscopic();
        createAval = gmp->GetCreateAvalancheMC();
        fVisualizeChamber = gmp->GetVisualizeChamber();
        fVisualizeSignal = gmp->GetVisualizeSignals();
        fVisualizeField = gmp->GetVisualizeField();
        driftRKF = gmp->GetDriftRKF();
        vAnodeWires = gmp->GetVoltageAnodeWires();
        vCathodePlane = gmp->GetVoltageCathodePlane();
        name="HeedDeltaElectronModel";
        InitialisePhysics();

        G4cout << "(Debug: HeedDeltaElectronModel.cc) The HeedDeltaElectronModel has been properly initialized..." << G4endl; 
    }

HeedDeltaElectronModel::~HeedDeltaElectronModel() {}


//This method is called in the DoIt-method in parent class HeedModel
void HeedDeltaElectronModel::Run(G4FastStep& fastStep,const G4FastTrack& fastTrack, G4String particleName, double ekin_eV, double t, double x_cm,
            double y_cm, double z_cm, double dx, double dy, double dz){

    G4double ekin_keV = ekin_eV / keV; // For the Transport functions

    G4cout << "(Debug: HeedDeltaElectronModel.cc) The energy here is: " << ekin_keV << " keV" << G4endl; 
    int nc = 0, ni=0;
    G4cout << "(Debug: HeedDeltaElectronModel.cc) Running interface..." << G4endl;
    if(particleName == "e-"){
        G4cout << "(Debug: HeedDeltaElectronModel.cc) Inside the electron case..." << G4endl;
        G4AutoLock lock(&aMutex);
        fTrackHeed->TransportDeltaElectron(x_cm, y_cm, z_cm, t, ekin_keV, dx, dy,
                                           dz, nc, ni);
        G4cout << "(Debug: HeedDeltaElectronModel.cc) Already transported electron..." << G4endl;
    }
    else{
        G4AutoLock lock(&aMutex);
        fTrackHeed->TransportPhoton(x_cm, y_cm, z_cm, t, ekin_keV, dx, dy,
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
        if(G4VVisManager::GetConcreteInstance() && cl % 100 == 0)
            G4cout << "(Debug: HeedDeltaElectronModel.cc) Now drifting..." << G4endl;
            Drift(xe,ye,ze,te);
    }
    PlotTrack();
    fastStep.KillPrimaryTrack();
    fastStep.ProposePrimaryTrackPathLength(0.0);
    fastStep.ProposeTotalEnergyDeposited(ekin_keV);

}

void HeedDeltaElectronModel::ProcessEvent(){

}

void HeedDeltaElectronModel::Reset(){
  
}


