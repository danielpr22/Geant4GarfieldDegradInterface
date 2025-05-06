#ifndef GasBoxSD_hh
#define GasBoxSD_hh

// Included from the loaded libraries (G4, ROOT, Garfield++, Degrad...)
#include "GenericSD.hh"
#include "G4String.hh"
#include "G4Region.hh"
#include "GasBoxHit.hh"
#include "GarfieldExcitationHit.hh"

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;


class GasBoxSD : public GenericSD {
	public:
	
	GasBoxSD(G4String);
	~GasBoxSD();
	
	virtual void 	Initialize (G4HCofThisEvent *);
	virtual void 	EndOfEvent (G4HCofThisEvent *);
	virtual G4bool ProcessHits(G4Step*, G4TouchableHistory*);
	virtual void DrawAll();
    void InsertGasBoxHit(const G4ThreeVector& position, double time) override {fGasBoxHitsCollection->insert(xh);};
    void InsertGarfieldExcitationHit(GarfieldExcitationHit* geh){fGarfieldExcitationHitsCollection->insert(geh);};
    
	private:
    GasBoxHitsCollection* fGasBoxHitsCollection;
    GarfieldExcitationHitsCollection* fGarfieldExcitationHitsCollection;
    G4int GBHCID; // For Gas Box Hits Collection ID
};

#endif

