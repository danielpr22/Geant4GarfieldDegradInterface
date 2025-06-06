/*
 * DegradModel.h
 *
 *  Created on: Apr 9, 2014
 *      Author: dpfeiffe
 * 
 *  Updated on: Jun 6, 2025
 * 		Update: Daniel Perales Rios
 */

#ifndef DEGRADMODEL_H_
#define DEGRADMODEL_H_

#include "GasModelParameters.hh"
#include "GasBoxSD.hh"
#include "DetectorMessenger.hh"

#include "G4ThreeVector.hh"
#include "G4VFastSimulationModel.hh"

class G4VPhysicalVolume;
class DetectorConstruction;
class GasBoxSD;


class DegradModel : public G4VFastSimulationModel{
	public:
		// Constructor and destructor
		DegradModel(GasModelParameters*, G4String, G4Region*,DetectorConstruction*,GasBoxSD*);
		~DegradModel();

		virtual G4bool IsApplicable(const G4ParticleDefinition&);
		virtual G4bool ModelTrigger(const G4FastTrack&);
		virtual void DoIt(const G4FastTrack&, G4FastStep&);
		inline G4bool FindParticleName(G4String s){ if(s == "e-") return true; return false; };
		inline void Reset(){ processOccured=false; };

		bool IsEventSuccessful() const { return isEventSuccessful; }; 
		void ResetEventSuccessfulFlag() { isEventSuccessful = false; };

	private:
		bool isEventSuccessful = false; 
		void GetElectronsFromDegrad(G4FastStep& fastStep,G4ThreeVector degradPos,G4double degradTime);
		GasModelParameters* fGasModelParameters;
		G4double thermalE;
		DetectorMessenger* messenger; 
		G4double voltageAnodeWires;
		G4double voltageCathodePlane;
		G4double photonEnergy;
		DetectorConstruction* detCon;
		GasBoxSD* fGasBoxSD;
		G4bool processOccured;
		G4int nbOfSecondaries; 
		G4int nbOfElectronsInBox; 
		G4int numberOfGases;
		G4String gasList;
		G4String gasPercentages; 
		G4double temperature; 
		G4double pressure; 
		G4double distanceAnodeCathodes;
		G4int secondaryElectronsPerPhoton;
        G4double driftDistanceThreshold; 
};

#endif
