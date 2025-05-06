#include "../include/AnodesHit.hh"

#include "G4UnitsTable.hh"
#include "G4VVisManager.hh"
#include "G4Circle.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"

G4ThreadLocal G4Allocator<AnodesHit>* AnodesHitAllocator;

AnodesHit::AnodesHit() : G4VHit(), fTime(-1),
   fPos(G4ThreeVector()){}

AnodesHit::~AnodesHit(){}

AnodesHit::AnodesHit(const AnodesHit& rhs) : G4VHit() {
    fPos= rhs.fPos;
    fTime=rhs.fTime;
}

const AnodesHit& AnodesHit::operator=(const AnodesHit& rhs){
    fPos= rhs.fPos;
    fTime=rhs.fTime;
    return *this;
}

G4int AnodesHit::operator==(const AnodesHit& rhs) const{
    return (this==&rhs) ? 1 : 0;
}

void AnodesHit::Draw()
{
  G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();
  if(pVVisManager)
  {
    G4Circle circle(fPos);
    circle.SetScreenSize(1.);
    circle.SetFillStyle(G4Circle::filled);
    G4Colour colour(0.,1.,0.);
    G4VisAttributes attribs(colour);
    circle.SetVisAttributes(attribs);
    pVVisManager->Draw(circle);
    
    //G4cout<<"DRAWING "<<fPos.getY()<<G4endl;
  }
}

void AnodesHit::Print(){
    G4cout << "(Debug: AnodesHit.cc) Printing hits..." << G4endl;
} 