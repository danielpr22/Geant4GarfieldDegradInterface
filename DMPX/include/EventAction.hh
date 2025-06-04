#ifndef EventAction_hh
#define EventAction_hh 1

// Included from the loaded libraries (G4, ROOT, Garfield++, Degrad...)
#include "G4UserEventAction.hh"
#include "G4ThreeVector.hh"
#include <vector>

class G4VPhysicalVolume;
class SteppingAction;
class G4Event;


class EventAction : public G4UserEventAction {
    public:
        EventAction();
        ~EventAction();

        void BeginOfEventAction(const G4Event *);
        void EndOfEventAction(const G4Event *);

        // Getter method for the energy of the primary and the distance from the anodes to the source
        G4double GetEnergyPrimary() const { return energyPrimary; } 
        G4double GetDistanceAndoesSource() const { return distanceAnodesSource; }
        G4int GetShotNumber() const { return shotNumber; } // Getter for the event counter 

        G4double energyPrimary; 
        G4double distanceAnodesSource; // Distance from the anodes to the source of photons
        G4int shotNumber;
        G4int numberOfEvents; // Counter for the number of secondary electrons generated in a given shot

        bool IsEventStarted() const { return eventStarted; }

    private:
        bool eventStarted; // Flag to indicate if a new event has started
};


#endif
