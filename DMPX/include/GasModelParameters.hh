#ifndef GasModelParameters_hh
#define GasModelParameters_hh

// Included from the loaded libraries (G4, ROOT, Garfield++, Degrad...)
#include "G4SystemOfUnits.hh"
#include "G4String.hh"
#include "G4ios.hh"
#include <map>
#include <iomanip>

class HeedDeltaElectronModelAnodes;
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
    
    void AddParticleNameHeedDeltaElectron(const G4String particleName,double ekin_min_keV,double ekin_max_keV);
    
    /*Getters and Setters*/
    //Name of the Magboltz file to be used (if needed)
    inline void SetGasFile(G4String s) { gasFile = s;};
    inline G4String GetGasFile() {return gasFile;};
    //Name of the Ion mobility file (if needed)
    inline void SetIonMobilityFile(G4String s) { ionMobFile = s; };
    inline G4String GetIonMobilityFile() {return  ionMobFile; };
    //Determines if the electrons are drifted, or only primary ionization is simulated
    inline void SetDriftElectrons(G4bool b) { driftElectrons = b; };
    inline bool GetDriftElectrons(){return driftElectrons;};
    inline void SetVoltageAnodeWires(G4double v){vAnodeWires = v;};
    inline double GetVoltageAnodeWires(){return vAnodeWires;};
    inline void SetVoltageCathodePlane(G4double v){vCathodePlane = v;};
    inline double GetVoltageCathodePlane(){return vCathodePlane;};
    inline void SetTrackMicroscopic(bool b){trackMicro=b;};
    inline bool GetTrackMicroscopic(){return trackMicro;};
    inline void SetCreateAvalancheMC(bool b){createAval=b;};
    inline bool GetCreateAvalancheMC(){return createAval;};
    inline void SetVisualizeChamber(bool b){fVisualizeChamber = b;};
    inline bool GetVisualizeChamber(){return fVisualizeChamber;};
    inline void SetVisualizeSignals(bool b){fVisualizeSignal = b;};
    inline bool GetVisualizeSignals(){return fVisualizeSignal;};
    inline void SetVisualizeField(bool b){fVisualizeField = b;};
    inline bool GetVisualizeField(){return fVisualizeField;};
    inline void SetDriftRKF(bool b){driftRKF=b;};
    inline bool GetDriftRKF(){return driftRKF;};
	inline void SetThermalEnergy(G4double d){thermalE=d;}
	inline G4double GetThermalEnergy(){return thermalE;};
    inline void SetNumberOfGases(int n){numberOfGases=n;};
    inline int GetNumberOfGases(){return numberOfGases;};
    inline void SetGasList(G4int g1, G4int g2, G4int g3, G4int g4, G4int g5, G4int g6) {
        gasList = std::to_string(g1) + "," + std::to_string(g2) + "," + std::to_string(g3) + "," +
                  std::to_string(g4) + "," + std::to_string(g5) + "," + std::to_string(g6);
    };
    inline G4String GetGasList(){return gasList;};
    inline void SetGasPercentages(G4double g1, G4double g2, G4double g3, G4double g4, G4double g5, G4double g6) {
        std::ostringstream oss; // This is due to the formatting of the gas percentages needed in Degrad
        oss << std::fixed << std::setprecision(1)
            << g1 << "," << g2 << "," << g3 << "," << g4 << "," << g5 << "," << g6;
        gasPercentages = oss.str();
        G4cout << "(Debug: GasModelParameters.cc) Gas percentages set to: " 
            << gasPercentages << G4endl;
    };
    inline G4String GetGasPercentages(){return gasPercentages;};
    inline G4double GetTemperature(){return temperature;};
    inline void SetTemperature(double n){temperature=n;};
    inline G4double GetDistanceAnodeCathodes(){return distanceAnodeCathodes;};
    inline void SetDistanceAnodeCathodes(double n){distanceAnodeCathodes=n;};
    inline void SetJumpDriftStepPoints(int n){jumpDriftStepPoints=n;};
    inline int GetJumpDriftStepPoints(){return jumpDriftStepPoints;};
    inline void SetSecondaryElectronsPerPhoton(int n){secondaryElectronsPerPhoton=n;};
    inline int GetSecondaryElectronsPerPhoton(){return secondaryElectronsPerPhoton;};
    
    inline MapParticlesEnergy GetParticleNamesHeedDeltaElectron(){return fMapParticlesEnergyHeedDeltaElectron;};

	private:
	GasModelParametersMessenger* fMessenger;
    MapParticlesEnergy fMapParticlesEnergyHeedDeltaElectron; // The particle map for the gas and the anodes will be the same

    G4String gasFile;
    G4String gasList;
    G4String gasPercentages;
    G4String ionMobFile;
    
    bool driftElectrons;
    bool trackMicro;
    bool createAval;
    bool fVisualizeChamber;
    bool fVisualizeSignal;
    bool fVisualizeField;
    bool driftRKF;
    
	G4double thermalE;
    G4double temperature; 
    G4double distanceAnodeCathodes; 
    G4int numberOfGases; 
    double vAnodeWires;
    double vCathodePlane;
    G4int jumpDriftStepPoints;
    G4int secondaryElectronsPerPhoton;
};

#endif
