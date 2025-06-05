#include "../include/GasModelParameters.hh"
#include "../include/DegradModel.hh"
#include "../include/GasModelParametersMessenger.hh"
#include "../include/DetectorConstruction.hh"
#include "../include/HeedDeltaElectronModel.hh"
#include "G4UnitsTable.hh"


GasModelParameters::GasModelParameters(){
	fMessenger = new GasModelParametersMessenger(this);
}


//Add particles (with energy range) to be included in the HeedDeltaElectron Model
void GasModelParameters::AddParticleNameHeedDeltaElectron(const G4String particleName,double ekin_min_keV,double ekin_max_keV){
    if (ekin_min_keV >= ekin_max_keV) {
        return;
    }

    fMapParticlesEnergyHeedDeltaElectron.insert(
        std::make_pair(particleName, std::make_pair(ekin_min_keV, ekin_max_keV)));
    
    G4cout << "(Debug: GasModelParameters.cc) HeedDeltaElectronModel: Particle added: " << G4BestUnit(ekin_min_keV, "Energy") 
    << " " << G4BestUnit(ekin_max_keV, "Energy") << G4endl;
}