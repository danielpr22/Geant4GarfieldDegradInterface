#ifndef GasBoxSD_hh
#define GasBoxSD_hh

#include "G4VSensitiveDetector.hh"
#include "G4String.hh"
#include "G4Region.hh"
#include "../include/GasBoxHit.hh"

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;


class GasBoxSD : public G4VSensitiveDetector{
	public:
	
	GasBoxSD(G4String);
	~GasBoxSD();
	
	virtual void 	Initialize (G4HCofThisEvent *);
	virtual void 	EndOfEvent (G4HCofThisEvent *);
	virtual G4bool ProcessHits(G4Step*, G4TouchableHistory*);
    void InsertGasBoxHit(GasBoxHit* gbh){fGasBoxHitsCollection->insert(gbh);};

	private:
	
	using GasBoxHitsCollection = G4THitsCollection<GasBoxHit>;
    GasBoxHitsCollection* fGasBoxHitsCollection;
    G4int GBHCID;	
};

#endif