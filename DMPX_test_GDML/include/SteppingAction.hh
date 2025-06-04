#ifndef SteppingAction_h
#define SteppingAction_h 1

#include <vector>

#include "G4UserSteppingAction.hh"
#include "G4Types.hh"
#include "G4String.hh"
#include "G4OpBoundaryProcess.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class SteppingAction : public G4UserSteppingAction {
 public:
  SteppingAction();
  ~SteppingAction(){};

  void UserSteppingAction(const G4Step *);
 
 private:
 	
};

#endif
