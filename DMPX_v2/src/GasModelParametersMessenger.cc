#include "../include/GasModelParametersMessenger.hh"
#include "../include/GasModelParameters.hh"
#include "../include/DegradModel.hh"

# include <iomanip> // For dealing with decimal precision 

#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4UIparameter.hh"
#include "G4Tokenizer.hh"
#include "G4UImanager.hh"


GasModelParametersMessenger::GasModelParametersMessenger(GasModelParameters* gm)
    : fGasModelParameters(gm) {
  GasModelParametersDir = new G4UIdirectory("/gasModelParameters/");
  GasModelParametersDir->SetGuidance("GasModelParameters specific controls");
  DegradDir = new G4UIdirectory("/gasModelParameters/degrad/");
  DegradDir->SetGuidance("Degrad specific controls");
  HeedDir = new G4UIdirectory("/gasModelParameters/heed/");
  HeedDir->SetGuidance("Heed specific controls");
  HeedDeltaElectronDir = new G4UIdirectory("/gasModelParameters/heed/heedinterface/");
  HeedDeltaElectronDir->SetGuidance("HeedDeltaElectron specific controls");

  addParticleHeedDeltaElectronCmd = new G4UIcommand("/gasModelParameters/heed/heeddeltaelectron/addparticle",this);
  addParticleHeedDeltaElectronCmd->SetGuidance("Set properties of the particle to be included");
  addParticleHeedDeltaElectronCmd->SetGuidance("[usage] /gasModelParameters/heed/heeddeltaelectron/addparticle P Emin Emax");
  addParticleHeedDeltaElectronCmd->SetGuidance("        P:(String) particle name (e-, e+, p, mu+, mu-, mu, pi,...");
  addParticleHeedDeltaElectronCmd->SetGuidance("        Emin:(double) Minimum energy for the model to be activated");
  addParticleHeedDeltaElectronCmd->SetGuidance("        Emax:(double Maximum energy for the model to be activated");

  G4UIparameter* paramHDE;
  paramHDE = new G4UIparameter("P",'s',false);
  paramHDE->SetDefaultValue("e-");
  addParticleHeedDeltaElectronCmd->SetParameter(paramHDE);
  paramHDE = new G4UIparameter("Emin",'d',true);
  paramHDE->SetDefaultValue("0.001");
  addParticleHeedDeltaElectronCmd->SetParameter(paramHDE);
  paramHDE = new G4UIparameter("Emax",'d',true);
  paramHDE->SetDefaultValue("1000.");
  addParticleHeedDeltaElectronCmd->SetParameter(paramHDE);

  gasFileCmd =  new G4UIcmdWithAString("/gasModelParameters/heed/gasfile",this);
  gasFileCmd->SetGuidance("Set name of the gas file");

  ionMobFileCmd =  new G4UIcmdWithAString("/gasModelParameters/heed/ionmobilityfile",this);
  ionMobFileCmd->SetGuidance("Set name of the ion mobility file");

  driftElectronsCmd = new G4UIcmdWithABool("/gasModelParameters/heed/drift",this);
  driftElectronsCmd->SetGuidance("true if ions and electrons are to be drifted in the electric field");

  driftRKFCmd = new G4UIcmdWithABool("/gasModelParameters/heed/driftRKF",this);
  driftRKFCmd->SetGuidance("true if runge kutta is used for the drift");

  createAvalCmd = new G4UIcmdWithABool("/gasModelParameters/heed/createAval",this);
  createAvalCmd->SetGuidance("true if monte carlo simulation of an avalanches is to be used");

  trackMicroCmd = new G4UIcmdWithABool("/gasModelParameters/heed/trackmicroscopic",this);
  trackMicroCmd->SetGuidance("true if microscopic tracking of the drift electrons/ions and avalanche is to be used");

  visualizeChamberCmd = new G4UIcmdWithABool("/gasModelParameters/heed/visualizechamber",this);
  visualizeChamberCmd->SetGuidance("true if visualization of the chamber configuration has to be shown");

  visualizeSignalsCmd = new G4UIcmdWithABool("/gasModelParameters/heed/visualizesignals",this);
  visualizeSignalsCmd->SetGuidance("true if signal on the pads have to be visualized");  

  visualizeFieldCmd = new G4UIcmdWithABool("/gasModelParameters/heed/visualizefield",this);
  visualizeFieldCmd->SetGuidance("true if the electric field has to be shown");

  voltageAnodeWiresCmd = new G4UIcmdWithADouble("/gasModelParameters/heed/voltageanodewire",this);
  voltageAnodeWiresCmd->SetGuidance("Set the voltage on the anode wire in V");

  voltageCathodePlaneCmd = new G4UIcmdWithADouble("/gasModelParameters/heed/voltagecathodeplane",this);
  voltageCathodePlaneCmd->SetGuidance("Set the voltage on the cathode plane in V");
    
  thermalEnergyCmd = new G4UIcmdWithADoubleAndUnit("/gasModelParameters/degrad/thermalenergy",this);
  thermalEnergyCmd->SetGuidance("Set the thermal energy to be used by degrad");

  numberOfGasesCmd = new G4UIcmdWithAnInteger("/gasModelParameters/degrad/numberofgases",this);
  numberOfGasesCmd->SetGuidance("Set the number of gases to be used by Degrad");

  // Gas list command
  gasListCmd = new G4UIcommand("/gasModelParameters/degrad/setGasList", this);
  gasListCmd->SetGuidance("Input the gas identifiers for Degrad.");

  // The following 6 parameters will store the gas identifiers for the gases in Degrad
  G4UIparameter* gas1 = new G4UIparameter("GAS1", 'd', false); 
  gas1->SetGuidance("First gas identifier in Degrad");
  gasListCmd->SetParameter(gas1);

  G4UIparameter* gas2 = new G4UIparameter("GAS2", 'd', false); 
  gas2->SetGuidance("Second gas identifier in Degrad");
  gasListCmd->SetParameter(gas2);

  G4UIparameter* gas3 = new G4UIparameter("GAS3", 'd', false); 
  gas3->SetGuidance("Third gas identifier in Degrad");
  gasListCmd->SetParameter(gas3);

  G4UIparameter* gas4 = new G4UIparameter("GAS4", 'd', false); 
  gas4->SetGuidance("Fourth gas identifier in Degrad");
  gasListCmd->SetParameter(gas4);

  G4UIparameter* gas5 = new G4UIparameter("GAS5", 'd', false);
  gas5->SetGuidance("Fifth gas identifier in Degrad");
  gasListCmd->SetParameter(gas5);

  G4UIparameter* gas6 = new G4UIparameter("GAS6", 'd', false);
  gas6->SetGuidance("Sixth gas identifier in Degrad");
  gasListCmd->SetParameter(gas6);

  // Gas percentages command
  gasPercentagesCmd = new G4UIcommand("/gasModelParameters/degrad/setGasPercentages", this);
  gasPercentagesCmd->SetGuidance("Input the molar gas percentages for Degrad.");

  // The following 6 parameters will store the gas percentages for the gases in Degrad
  G4UIparameter* gas1Percentage = new G4UIparameter("GAS1", 'd', false);
  gas1Percentage->SetGuidance("First gas percentage in Degrad");
  gasPercentagesCmd->SetParameter(gas1Percentage);

  G4UIparameter* gas2Percentage = new G4UIparameter("GAS2", 'd', false);
  gas2Percentage->SetGuidance("Second gas percentage in Degrad");
  gasPercentagesCmd->SetParameter(gas2Percentage);

  G4UIparameter* gas3Percentage = new G4UIparameter("GAS3", 'd', false);
  gas3Percentage->SetGuidance("Third gas percentage in Degrad");
  gasPercentagesCmd->SetParameter(gas3Percentage);

  G4UIparameter* gas4Percentage = new G4UIparameter("GAS4", 'd', false);
  gas4Percentage->SetGuidance("Fourth gas percentage in Degrad");
  gasPercentagesCmd->SetParameter(gas4Percentage);

  G4UIparameter* gas5Percentage = new G4UIparameter("GAS5", 'd', false);
  gas5Percentage->SetGuidance("Fifth gas percentage in Degrad");
  gasPercentagesCmd->SetParameter(gas5Percentage);

  G4UIparameter* gas6Percentage = new G4UIparameter("GAS6", 'd', false);
  gas6Percentage->SetGuidance("Sixth gas percentage in Degrad");
  gasPercentagesCmd->SetParameter(gas6Percentage);

  temperatureCmd = new G4UIcmdWithADoubleAndUnit("/gasModelParameters/degrad/temperature",this);
  temperatureCmd->SetGuidance("Set the temperature to be used by Degrad");

  distanceAnodeCathodesCmd = new G4UIcmdWithADoubleAndUnit("/gasModelParameters/degrad/distanceanodecathodes",this);
  distanceAnodeCathodesCmd->SetGuidance("Set the distance between the anodes and the cathodes in cm"); 
  
  jumpDriftStepPointsCmd = new G4UIcmdWithAnInteger("/gasModelParameters/heed/jumpDriftStepPoints",this);
  jumpDriftStepPointsCmd->SetGuidance("Set the number of drift step points to be skipped in the visualization");

  secondaryElectronsPerPhotonCmd = new G4UIcmdWithAnInteger("/gasModelParameters/degrad/secondaryElectronsPerPhoton",this);
  secondaryElectronsPerPhotonCmd->SetGuidance("Set the number of secondary electrons to be calculated in the Degrad avalanche");

}



GasModelParametersMessenger::~GasModelParametersMessenger() {
  delete GasModelParametersDir;
  delete DegradDir;
  delete HeedDir;
  delete HeedDeltaElectronDir;
  delete addParticleHeedDeltaElectronCmd;
  delete gasFileCmd;
  delete ionMobFileCmd;
  delete driftElectronsCmd;
  delete driftRKFCmd;
  delete createAvalCmd;
  delete trackMicroCmd;
  delete visualizeChamberCmd;
  delete visualizeSignalsCmd;
  delete visualizeFieldCmd;
  delete voltageAnodeWiresCmd;
  delete voltageCathodePlaneCmd;
  delete thermalEnergyCmd;
  delete numberOfGasesCmd; 
  delete gasListCmd;
  delete gasPercentagesCmd;
  delete temperatureCmd;
  delete distanceAnodeCathodesCmd;
  delete jumpDriftStepPointsCmd;
  delete secondaryElectronsPerPhotonCmd;
}


void GasModelParametersMessenger::SetNewValue(G4UIcommand* command, G4String newValues) {

    if(command == thermalEnergyCmd){
      fGasModelParameters->SetThermalEnergy(thermalEnergyCmd->GetNewDoubleValue(newValues));
    }
    else if(command == addParticleHeedDeltaElectronCmd) {
	  	AddParticleHeedDeltaElectronCommand(newValues);
    }
	  else if(command == gasFileCmd) {
	  	fGasModelParameters->SetGasFile(newValues);
	  }
	  else if(command == ionMobFileCmd) {
	  	fGasModelParameters->SetIonMobilityFile(newValues);
	  }
	  else if(command == driftElectronsCmd) {
	  	fGasModelParameters->SetDriftElectrons(driftElectronsCmd->GetNewBoolValue(newValues));
	  }
	  else if(command == driftRKFCmd) {
	  	fGasModelParameters->SetDriftRKF(driftRKFCmd->GetNewBoolValue(newValues));
	  }
	  else if(command == createAvalCmd) {
	  	fGasModelParameters->SetCreateAvalancheMC(createAvalCmd->GetNewBoolValue(newValues));
	  }
	  else if(command == trackMicroCmd) {
	  	fGasModelParameters->SetTrackMicroscopic(trackMicroCmd->GetNewBoolValue(newValues));
	  }
	  else if(command == visualizeChamberCmd) {
	  	fGasModelParameters->SetVisualizeChamber(visualizeChamberCmd->GetNewBoolValue(newValues));
	  }
	  else if(command == visualizeSignalsCmd){
	  	fGasModelParameters->SetVisualizeSignals(visualizeSignalsCmd->GetNewBoolValue(newValues));
	  }
	  else if(command == visualizeFieldCmd){
	  	fGasModelParameters->SetVisualizeField(visualizeFieldCmd->GetNewBoolValue(newValues));
	  }
	  else if(command == voltageAnodeWiresCmd){
	  	fGasModelParameters->SetVoltageAnodeWires(voltageAnodeWiresCmd->GetNewDoubleValue(newValues));
	  }
	  else if(command == voltageCathodePlaneCmd){
	  	fGasModelParameters->SetVoltageCathodePlane(voltageCathodePlaneCmd->GetNewDoubleValue(newValues));
	  }
    else if(command == numberOfGasesCmd){
      fGasModelParameters->SetNumberOfGases(numberOfGasesCmd->GetNewIntValue(newValues));
    }
    else if(command == gasListCmd){
      G4Tokenizer next(newValues);
      G4int gas1 = StoD(next());
      G4int gas2 = StoD(next());
      G4int gas3 = StoD(next());
      G4int gas4 = StoD(next());
      G4int gas5 = StoD(next());
      G4int gas6 = StoD(next());
      fGasModelParameters->SetGasList(gas1, gas2, gas3, gas4, gas5, gas6);
      G4cout << "(Debug: GasModelParametersMessenger.cc) Gas list set to: "
           << gas1 << " " << gas2 << " " << gas3 << " " << gas4 << " " << gas5 << " " << gas6 << G4endl;
    }
    else if(command == gasPercentagesCmd){
      G4Tokenizer next(newValues);
      G4double gas1Percentage = StoD(next());
      G4double gas2Percentage = StoD(next());
      G4double gas3Percentage = StoD(next());
      G4double gas4Percentage = StoD(next());
      G4double gas5Percentage = StoD(next());
      G4double gas6Percentage = StoD(next());
      fGasModelParameters->SetGasPercentages(gas1Percentage, gas2Percentage, gas3Percentage, 
        gas4Percentage, gas5Percentage, gas6Percentage);
    }
    else if(command == temperatureCmd) {
      fGasModelParameters->SetTemperature(temperatureCmd->GetNewDoubleValue(newValues));
    }
    else if(command == distanceAnodeCathodesCmd) {
      fGasModelParameters->SetDistanceAnodeCathodes(distanceAnodeCathodesCmd->GetNewDoubleValue(newValues));
    }
    else if(command == jumpDriftStepPointsCmd) {
      fGasModelParameters->SetJumpDriftStepPoints(jumpDriftStepPointsCmd->GetNewIntValue(newValues));
    } 
    else if(command ==  secondaryElectronsPerPhotonCmd) {
      fGasModelParameters->SetSecondaryElectronsPerPhoton(secondaryElectronsPerPhotonCmd->GetNewIntValue(newValues));
    } 
    else {
      G4cerr << "(Debug: GasModelParametersMessenger.cc) GasModelParametersMessenger::"
      << "SetNewValue: Unknown command" << G4endl;
    }

}


void GasModelParametersMessenger::AddParticleHeedDeltaElectronCommand(G4String newValues){
	ConvertParameters(newValues);
	fGasModelParameters->AddParticleNameHeedDeltaElectron(fParticleName,fEmin/keV,fEmax/keV);
}

void GasModelParametersMessenger::ConvertParameters(G4String newValues){
	G4Tokenizer next( newValues );
	fParticleName = next();
	G4String Semin = next();
	if(Semin.empty()){
	 	fEmin = 1. *keV;
	 	fEmax = 1. *GeV;
	}
	else{
		fEmin = StoD(Semin);
		G4String Semax = next();
		if(Semax.empty())
		 	fEmax = 1.*GeV;
		else
			fEmax = StoD(Semax);
	}
}

