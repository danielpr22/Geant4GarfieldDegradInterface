/*
 * HeedDeltaElectronModel.h
 *
 *  Created on: May 6, 2025
 *      Author: Daniel Perales Rios
 */

#ifndef HEEDINTERFACEMODEL_H_
#define HEEDINTERFACEMODEL_H_

// Included from the current project
#include "HeedModel.hh"

class G4VPhysicalVolume;
class DetectorConstruction;
class DetectorMessenger;
class HeedDeltaElectronMessenger;
class GasModelParameters;
class GasBoxSD;
class G4FastStep;
class G4FastTrack;

typedef std::pair<double, double> EnergyRange_keV;
typedef std::multimap<const G4String, EnergyRange_keV> MapParticlesEnergy;

class HeedDeltaElectronModel : public HeedModel {
 public:
  //-------------------------
  // Constructor, destructor
  //-------------------------
  HeedDeltaElectronModel(GasModelParameters *,G4String, G4Region*,DetectorConstruction*, GasBoxSD*);
  ~HeedDeltaElectronModel();
 

 private:
  virtual void Run(G4FastStep& fastStep,const G4FastTrack& fastTrack, G4String particleName, double ekin_eV, double t, double x_cm,
            double y_cm, double z_cm, double dx, double dy, double dz);
};

#endif /* HeedDeltaElectronModel_H_ */

