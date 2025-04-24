#include "SteppingAction.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VTouchable.hh"
#include "G4Step.hh"
#include "DetectorConstruction.hh"
#include "G4Track.hh"
#include "G4OpticalPhoton.hh"
#include "G4StepPoint.hh"
#include "G4SDManager.hh"
#include "GasBoxSD.hh"
#include "G4EqMagElectricField.hh"

G4ElectricField* pEMfield;


SteppingAction::SteppingAction(){

}


void SteppingAction::UserSteppingAction(const G4Step *aStep) {
    G4Track* track = aStep->GetTrack();
    G4ThreeVector pos = track->GetPosition();
    
    G4double xyz[4] = { pos.x(), pos.y(), pos.z(), 0. };
    G4double fieldVal[6] = {0., 0., 0., 0., 0., 0.};

    pEMfield->GetFieldValue(xyz, fieldVal);

    G4cout << "Electric field at step position (" 
           << pos.x()/mm << ", "
           << pos.y()/mm << ", "
           << pos.z()/mm << ") = ("
           << fieldVal[0]/(volt/m) << ", "
           << fieldVal[1]/(volt/m) << ", "
           << fieldVal[2]/(volt/m) << ") V/m"
           << G4endl;
	
}
