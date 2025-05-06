#ifndef DetectorSD_hh
#define DetectorSD_hh

#include "GenericSD.hh"
#include "G4String.hh"
#include "G4Region.hh"
#include "GasBoxHit.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;

class AnodeSD : public GenericSD {
	public:
	
	AnodeSD(G4String);
	~AnodeSD();
	
	virtual void 	Initialize (G4HCofThisEvent *);
	virtual void 	EndOfEvent (G4HCofThisEvent *);
	virtual G4bool ProcessHits(G4Step*, G4TouchableHistory*);
	
	private:
	
    GasBoxHitsCollection* fAnodeHitsCollection;
    G4int AWHCID;
};

#endif

