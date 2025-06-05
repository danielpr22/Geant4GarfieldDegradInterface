#ifndef GasModelParametersMessenger_h
#define GasModelParametersMessenger_h 1

#include "G4SystemOfUnits.hh"
#include "G4UImessenger.hh"

class G4UIcommand;
class GasModelParameters;
class G4UIdirectory;
class G4UIcmdWithAString;
class G4UIcmdWithABool;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithoutParameter;
class G4UIcmdWithADouble;
class G4UIcmdWithAnInteger;


class GasModelParametersMessenger : public G4UImessenger{
    public:
        GasModelParametersMessenger(GasModelParameters*);
        ~GasModelParametersMessenger();

        void SetNewValue(G4UIcommand*, G4String);

    private:
        void AddParticleHeedDeltaElectronCommand(G4String newValues);
        void AddParticleHeedNewTrackCommand(G4String newValues);
        void ConvertParameters(G4String newValues);

        // Instance of GasModelParameters
        GasModelParameters* fGasModelParameters;

        // Directories
        G4UIdirectory* GasModelParametersDir;
        G4UIdirectory* DegradDir;
        G4UIdirectory* HeedDir;
        G4UIdirectory* HeedDeltaElectronDir;

        // Particle map commands and variables
        G4UIcommand* addParticleDegradCmd;
        G4UIcommand* addParticleHeedDeltaElectronCmd;
        G4String fParticleName;
        G4double fEmin;
        G4double fEmax;

        // Gas and ion mobility files commands, gas settings commands
        G4UIcmdWithAString* gasFileCmd;
        G4UIcmdWithAString* ionMobFileCmd;
        G4UIcmdWithAnInteger* numberOfGasesCmd;
        G4UIcommand* gasListCmd; 
        G4UIcommand* gasPercentagesCmd;

        // Drifting commands
        G4UIcmdWithABool* driftElectronsCmd;
        G4UIcmdWithABool* driftRKFCmd;
        G4UIcmdWithABool* trackMicroCmd;
        G4UIcmdWithABool* createAvalCmd;
        G4UIcmdWithAnInteger* secondaryElectronsPerPhotonCmd;
        G4UIcmdWithAnInteger* jumpDriftStepPointsCmd;

        // Visualization commands
        G4UIcmdWithABool* visualizeChamberCmd;
        G4UIcmdWithABool* visualizeSignalsCmd;
        G4UIcmdWithABool* visualizeFieldCmd;

        // Anodes and cathodes commands
        G4UIcmdWithADouble* voltageAnodeWiresCmd;
        G4UIcmdWithADouble* voltageCathodePlaneCmd;
        G4UIcmdWithADoubleAndUnit* thermalEnergyCmd;
        G4UIcmdWithADoubleAndUnit* distanceAnodeCathodesCmd;

        // Temperature command
        G4UIcmdWithADoubleAndUnit* temperatureCmd;
};

#endif
