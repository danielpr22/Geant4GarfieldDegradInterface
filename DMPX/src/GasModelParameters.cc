#include "../include/GasModelParameters.hh"
#include "../include/HeedDeltaElectronModel.hh"
#include "../include/HeedNewTrackModel.hh"
#include "../include/GasModelParametersMessenger.hh"
#include "../include/DetectorConstruction.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

GasModelParameters::GasModelParameters() {
	fMessenger = new GasModelParametersMessenger(this);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//Add particles (with energy range) to be included in the HeedNewTrack Model
void GasModelParameters::AddParticleNameHeedNewTrack(const G4String particleName,
    double ekin_min_keV,double ekin_max_keV) {
    if (ekin_min_keV >= ekin_max_keV) {
        return;
    }
    fMapParticlesEnergyHeedNewTrack.insert(std::make_pair(particleName, 
        std::make_pair(ekin_min_keV, ekin_max_keV)));
    G4cout << "(Debug: GasModelParameters.cc) Particle added in HeedNewTrack: " << particleName 
    << " | " << ekin_min_keV << " to " << ekin_max_keV << " keV" << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//Add particles (with energy range) to be included in the HeedDeltaElectron Model
void GasModelParameters::AddParticleNameHeedDeltaElectron(const G4String particleName,
    double ekin_min_keV,double ekin_max_keV) {
    if (ekin_min_keV >= ekin_max_keV) {
        return;
    }
    fMapParticlesEnergyHeedDeltaElectron.insert(std::make_pair(particleName, 
        std::make_pair(ekin_min_keV, ekin_max_keV)));

    G4cout << "(Debug: GasModelParameters.cc) Particle added in HeedDeltaElectron: " << particleName 
    << " | " << ekin_min_keV << " to " << ekin_max_keV << " keV" << G4endl;
}
