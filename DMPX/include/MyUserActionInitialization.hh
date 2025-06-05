#ifndef MyUserActionInitialization_hh
#define MyUserActionInitialization_hh

// Included from the loaded libraries (G4, ROOT, Garfield++, Degrad...)
#include "G4VUserActionInitialization.hh"

class DetectorConstruction;
class PhysicsList;

class MyUserActionInitialization : public G4VUserActionInitialization{
	public:
        MyUserActionInitialization();
        ~MyUserActionInitialization();
        
        void Build() const;
        void BuildForMaster() const;	
};

#endif
