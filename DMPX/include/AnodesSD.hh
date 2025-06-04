#ifndef AnodesSD_hh
#define AnodesSD_hh

#include "G4VSensitiveDetector.hh"
#include "G4String.hh"
#include "G4Region.hh"
#include "AnodesHit.hh"


class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;


class AnodesSD : public G4VSensitiveDetector{
	public:
	
	AnodesSD(G4String);
	~AnodesSD();
	
	virtual void 	Initialize (G4HCofThisEvent *);
	virtual void 	EndOfEvent (G4HCofThisEvent *);
	virtual G4bool ProcessHits(G4Step*, G4TouchableHistory*);
	
	private:
	
    AnodesHitsCollection* fAnodesHitsCollection;
    G4int AHCID;
    
	
};

#endif
