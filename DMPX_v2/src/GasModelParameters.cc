#include "../include/GasModelParameters.hh"
#include "../include/DegradModel.hh"
#include "../include/GasModelParametersMessenger.hh"
#include "../include/DetectorConstruction.hh"
#include "../include/HeedDeltaElectronModel.hh"


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
    
    G4cout << "(Debug: GasModelParameters.cc) HeedDeltaElectronGas/Anodes: Particle added: " << ekin_min_keV << " " << ekin_max_keV << G4endl;
}