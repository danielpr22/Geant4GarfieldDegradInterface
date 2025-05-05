#ifndef GasModelParameters_hh
#define GasModelParameters_hh

#include "G4SystemOfUnits.hh"
#include "G4String.hh"
#include <map>

class HeedDeltaElectronModel;
class HeedNewTrackModel;
class DegradModel;
class GasModelParametersMessenger;
class DetectorConstruction;
class G4String;

// Map of particles and associated energies
typedef std::pair<double, double> EnergyRange_keV;
typedef std::multimap<const G4String, EnergyRange_keV> MapParticlesEnergy;

class GasModelParameters{
	public:
	
	GasModelParameters();
	~GasModelParameters();
    
	inline void SetThermalEnergy(G4double d){thermalE=d;}
	inline G4double GetThermalEnergy(){return thermalE;};
    
	
	private:
	GasModelParametersMessenger* fMessenger;
    
    G4String gasFile;
    G4String ionMobFile;
	G4double thermalE;
};

#endif
