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

 public:
  void BeginOfEventAction(const G4Event *);
  void EndOfEventAction(const G4Event *);
  

 private:
	
};


#endif
