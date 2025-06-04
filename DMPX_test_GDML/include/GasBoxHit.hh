#ifndef GASBOXHIT_HH
#define GASBOXHIT_HH

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class GasBoxHit : public G4VHit {
    
public:
    GasBoxHit();
    virtual ~GasBoxHit();
    GasBoxHit(const GasBoxHit &);
    
    /*
    The copy assignment operator is invoked when an existing object is 
    assigned the values of another object of the same class. For example:
    
    GasBoxHit hit1;
    GasBoxHit hit2;
    hit2 = hit1; // Calls the copy assignment operator
    */
    const GasBoxHit& operator=(const GasBoxHit&);
    G4int operator==(const GasBoxHit&) const;
    
    inline void* operator new(size_t);
    inline void  operator delete(void*);
	
	virtual void Draw();
    virtual void Print();
    
    G4ThreeVector GetPos(){return fPos;};
    G4double GetTime(){return fTime;};
    
    void SetPos(G4ThreeVector xyz){ fPos = xyz; };
    void SetTime(G4double t){ fTime = t; };
    
    
private:
    G4double      fTime;
    G4ThreeVector fPos;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

using GasBoxHitsCollection=G4THitsCollection<GasBoxHit>;

extern G4ThreadLocal G4Allocator<GasBoxHit>* GasBoxHitAllocator;

inline void* GasBoxHit::operator new(size_t) {
  if (!GasBoxHitAllocator) {
         GasBoxHitAllocator = new G4Allocator<GasBoxHit>;
  }
  return (void*)GasBoxHitAllocator->MallocSingle();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

inline void GasBoxHit::operator delete(void *aHit) {
    GasBoxHitAllocator->FreeSingle((GasBoxHit*) aHit);
}

#endif
