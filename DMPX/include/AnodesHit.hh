#ifndef AnodesHit_HH
#define AnodesHit_HH

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

class AnodesHit : public G4VHit {
    public:
        AnodesHit();
        virtual ~AnodesHit();
        AnodesHit(const AnodesHit &);
        
        const AnodesHit& operator=(const AnodesHit&);
        G4int operator==(const AnodesHit&) const;
        
        inline void* operator new(size_t);
        inline void  operator delete(void*);
        
        virtual void Draw();
        virtual void Print();
        
        G4ThreeVector GetPos(){ return fPos; };
        G4double GetTime(){ return fTime; };
        
        void SetPos(G4ThreeVector xyz){ fPos = xyz; };
        void SetTime(G4double t){ fTime = t; };
        
    private:
        G4double fTime;
        G4ThreeVector fPos;
};

using AnodesHitsCollection=G4THitsCollection<AnodesHit>;

extern G4ThreadLocal G4Allocator<AnodesHit>* AnodesHitAllocator;

inline void* AnodesHit::operator new(size_t){
  if (!AnodesHitAllocator){
    AnodesHitAllocator = new G4Allocator<AnodesHit>;
  }
  return (void*)AnodesHitAllocator->MallocSingle();
}

inline void AnodesHit::operator delete(void *aHit){
    AnodesHitAllocator->FreeSingle((AnodesHit*) aHit);
}

#endif