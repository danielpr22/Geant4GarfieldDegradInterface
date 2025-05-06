#ifndef GENERICSD_HH
#define GENERICSD_HH

#include "G4ThreeVector.hh"

class GenericSD {
public:
    virtual ~GenericSD() {}
    virtual void InsertGasBoxHit(const G4ThreeVector& position, double time) = 0;
};

#endif // GENERICSD_HH